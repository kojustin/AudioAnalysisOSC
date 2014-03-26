#include "cinder/Cinder.h"

#include "cinder/app/AppBasic.h"
//#include "cinder/app/AppNative.h"
#include "cinder/audio/FftProcessor.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Text.h"

#include "cinder/Utilities.h"

typedef ci::app::AppBasic AppBase;

//#include "cinder/audio/Input.h"
//#include <iostream>
#include <vector>

#include "Wave.h"
#include "Tap.h"

using namespace ci;
using namespace ci::app;

class AudioAnalysisOSCApp : public AppBase {
public:
    
    void prepareSettings( Settings *settings);
	void setup();
	void update();
	void draw();
    void keyDown(KeyEvent e);
    
    void displayText();
    void checkLookup(std::vector<int> &results, int a, int b, int c, int d, float res);
    
    // Audio Variables ////////////////////////////////////////////
    
	audio::Input mInput;
    
	audio::PcmBuffer32fRef mPcmBuffer;
    
    Wave waves [4];
    
    Boolean live;
    Boolean pauseEnabled;
    Boolean delay;
    
    uint16_t  channels = 0;
    
    // reolutions are now set based on the maximum time difference in x and y, this essentially means maximum accuracy but you cannot change it
    // for 105cm x 69cm we use 140 x 76 at 96,000hz sample rate
    
    // 246x87 for wall
    const static int resolutionX = 125;
    const static int resolutionY = 80;
    float lookup [resolutionX][resolutionY][4];
    
    // Font Setup ///////////////////////////////////////////////////
    
    Font				mFont;
	gl::TextureFontRef	mTextureFont;
    
    // Graphics Setup ///////////////////////////////////////////////
    
    std::vector<Tap> taps;
    float fade = 0.0f;
    int xMax = resolutionX;
    int yMax = resolutionY;
    
    
};

void AudioAnalysisOSCApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 1280, 800 );
    //settings->setFrameRate( 60.0f );
}

void AudioAnalysisOSCApp::setup()
{
    gl::enableAlphaBlending();
    
    // Audio Setup ////////////////////////////////////////////////////////////
    
	//iterate input devices and print their names to the console
	const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin(); iter != devices.end(); ++iter ) {
		console() << (*iter)->getName() << std::endl;
	}
    
	//initialize the audio Input, using the default input device
	mInput = audio::Input();
	
	//tell the input to start capturing audio
	mInput.start();
    
    channels = mInput.getChannelCount();
    console() << channels << std::endl;
    
    // initialise the Waves
    
    waves[0] =  Wave( mInput, audio::CHANNEL_FRONT_LEFT , 200.0f);
    waves[1] =  Wave( mInput, audio::CHANNEL_FRONT_RIGHT , 200.0f);
    waves[2] = Wave( mInput, audio::CHANNEL_BACK_LEFT, 200.0f );
    waves[3] = Wave( mInput, audio::CHANNEL_BACK_RIGHT, 200.0f );
    
    live = true;
    pauseEnabled = true;
    
    
    // generate lookup table
    
    for (int x = 0; x < resolutionX; x++){
        for (int y = 0; y < resolutionY; y++){
            
            // note the sqrts ipmlicitly convert an int to a double so (float) is required
            
            lookup[x][y][0] = (float)sqrt(x*x + y*y);
            lookup[x][y][1] = (float)sqrt( (resolutionX - x)*(resolutionX - x) + y*y);
            lookup[x][y][2] = (float)sqrt( (resolutionX - x)*(resolutionX - x) + (resolutionY - y)*(resolutionY - y) );
            lookup[x][y][3] = (float)sqrt( x*x + (resolutionY - y)*(resolutionY - y) );
            
            // find index that was the smallest value
            
            int s = 0;
            int minIdx = resolutionX*3;  // this assumes that the aspect ratio will never exceed 3:1 change this if needed
            
            for (int i = 0; i < 4; i++){
                if (lookup[x][y][i] < minIdx){
                    minIdx = lookup[x][y][i];
                    s = i;
                }
            }
            
            // subtract it from the others to get relative value
            
            float smallest = lookup[x][y][s];
            
            for (int i = 0; i < 4; i++){
                lookup[x][y][i] -= smallest;
            }
        }
    }
    
    
    // Font Setup /////////////////////////////////////////////////////////////////
    
    mFont = Font( "Consolas", 15 );
	mTextureFont = gl::TextureFont::create( mFont );
    
}

void AudioAnalysisOSCApp::update()
{
    mPcmBuffer = mInput.getPcmBuffer();
	if( ! mPcmBuffer ) {
		return;
	}
    
    // updates the number of buffer samples to use in wave.update()
    uint32_t bufferSamples = mPcmBuffer->getSampleCount();
    
    // if the program is live, update the contents of the waves
    if (live || !pauseEnabled){
        for (int i = 0; i < channels; i++){
            waves[i].update(mInput, bufferSamples);
        }
    }
    
    int s = 0;
    int minIdx = 3000;
    
    // loop finds the wave index that arrived first
    
    for (unsigned i = 0; i < channels; i++){
        if (waves[i].startIndex < minIdx){
            minIdx = waves[i].startIndex;
            s = i;
        }
    }
    
    Wave &earliest = waves[s];
    waves[s].relativeStart = 0;
    
    // sets all the start indexes relative to the first one
    for (unsigned i = 0; i < channels; i++){
        if (i != s){
            waves[i].relativeStart = waves[i].startIndex - earliest.startIndex;
        }
    }
    
    if (live){
        
        int peakCount = 0;
        
        // check to see if any of the waves have peaked
        
        for (int i = 0; i < channels; i++){
            
            if (waves[i].peaked) peakCount ++;
            
        }
        
        if (peakCount > 0){
            live = false;
            
            // THIS IMPLIES A TAP HAS BEEN REGISTERED
            
            float a = waves[0].relativeStart;
            float b = waves[1].relativeStart;
            float c = waves[2].relativeStart;
            float d = waves[3].relativeStart;
            
            std::vector<int> posVals;
            posVals.clear();
            
            checkLookup(posVals, a, b, c, d, resolutionX); // uses the largest resolution (default x) for distance measurement
            
            float triScaleX = (float)(getWindowWidth()/(float)resolutionX);
            float triScaleY = (float)(getWindowHeight()/(float)resolutionY);
            
            Vec2f pos = Vec2f( (float)posVals[0]*triScaleX + 50, (float)posVals[1]*triScaleY);
            
            
            // create new tap
            
            Tap tap = Tap( pos, abs(waves[s].max)) ;
            
//            if (waves[0].aveFreq > 20 ||
//                waves[1].aveFreq > 20 ||
//                waves[2].aveFreq > 20 ||
//                waves[3].aveFreq > 20 ){
//                tap.mode = 1;
//            } else {
//                tap.mode = 0;
//            }
            
            taps.push_back( tap );
            
            
            
        }
    }
    
    // this is a check for the laptop mic input difference between channel arrival times (it should be zero)
    //    int diff = abs(waves[0].startIndex - waves[1].startIndex);
    //    assert( diff < 5 );
    
}

void AudioAnalysisOSCApp::draw()
{
    
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
    
    glPushMatrix();
    glTranslatef(0.0f, 0.5f*getWindowHeight()/channels , 0.0f);
    
    // update the waves
    
    for (int i = 0; i < channels; i++){
        
        waves[i].drawWave(& live);
        waves[i].drawFft(2000.0f);
        
        glTranslatef(0.0f, getWindowHeight()/channels, 0.0f);
    }
    
    glPopMatrix();
    
    // draw the time bars
    
    gl::color(0.0f, 0.3f, 0.4f);
    
    for (int i = 0; i < channels; i++){
        gl::drawLine( Vec2f(waves[i].startPt.x, 0.0f), Vec2f(waves[i].startPt.x, cinder::app::getWindowHeight()) );
        gl::drawLine( Vec2f(0.0f, waves[i].startPt.y), Vec2f(cinder::app::getWindowWidth(), waves[i].startPt.y) );
    }
    
    displayText();
    
    // fade out the background
    
    Rectf fadeMask = Rectf( Vec2f(0.0f, 0.0f), Vec2f(getWindowWidth(), getWindowHeight()));
    
    gl::color( 0.0f, 0.0f, 0.0f, fade );
    gl::drawSolidRect(fadeMask);
    
    // display all taps
    
    std::vector<int> removals;
    
    for ( unsigned i = 0; i < taps.size(); i++ ){
        taps[i].run();
        
        if (taps[i].t > 1) removals.push_back( i );
    }
    
    for ( unsigned i = 0; i < removals.size(); i++ ){
        taps.erase( taps.begin()+removals[i] );
    }
}


void AudioAnalysisOSCApp::keyDown(KeyEvent e){
    
    if (e.getChar() == 's') {
        live = true;
    }
    
    if (e.getChar() == 'a') {
        pauseEnabled = !pauseEnabled;
    }
    
    if (e.getChar() == '-') {
        Wave::delayThresh *= 0.5f;
    }
    
    if (e.getChar() == '=') {
        Wave::delayThresh *= 2.0f;
    }
    
    if (e.getChar() == 'x') {
        xMax = abs(waves[0].startIndex - waves[1].startIndex);
    }
    
    if (e.getChar() == 'y') {
        yMax = abs(waves[1].startIndex - waves[2].startIndex);
    }
    
    if (e.getChar() == 'm') {
        fade+=0.2f;
    }
    
    if (e.getChar() == 'n') {
        fade-=0.2f;
    }
    
    if (e.getChar() == 'f') {
        console() << "freq: " << waves[0].aveFreq << std::endl;
        console() << "max: " << waves[0].max << std::endl;
        console() << "start: " << waves[0].startIndex << std::endl;
        console() << "attack: " << waves[0].attack << std::endl;
    }
    
}

void AudioAnalysisOSCApp::displayText(){
    
    glPushMatrix();
    
    glTranslatef(20.0f, 0.5f*getWindowHeight()/channels + 15.0f, 0.0f);
    
    gl::color( Color::white() );
    
    // input layer
    
    glPushMatrix();
    
    for (unsigned w = 0; w < channels; w++){
        mTextureFont->drawString(  "Relative Delay: " + toString(waves[w].relativeStart ), Vec2f( 0, 0 ) );
        mTextureFont->drawString(  "Max Volume: " + toString(floor(abs(waves[w].max)) ), Vec2f( 0, 15 ) );
        mTextureFont->drawString(  "Average Freq Band: " + toString(floor(abs(waves[w].aveFreq)) ), Vec2f( 0, 30 ) );
        mTextureFont->drawString(  "Attack: " + toString(floor(abs(waves[w].attack)) ), Vec2f( 0, 45 ) );
        
        glTranslatef(0.0f, getWindowHeight()/channels, 0.0f);
    }
    glPopMatrix();
    
    // output layer
    
    glTranslatef(950.0f, -50.0f, 0.0f);
    
    
    if (pauseEnabled) mTextureFont->drawString( "Pausing Enabled", Vec2f( 150.0f, 0.0f ) );
    if (pauseEnabled) mTextureFont->drawString( toString(Wave::delayThresh / 3.0f) + "sec delayTime", Vec2f( 150.0f, 15.0f ) );
    
    mTextureFont->drawString( toString(xMax) + " = xMax", Vec2f( 150.0f, 40.0f ) );
    mTextureFont->drawString( toString(yMax) + " = yMax", Vec2f( 150.0f, 60.0f ) );
    
    
    glPopMatrix();
    
}

void AudioAnalysisOSCApp::checkLookup(std::vector<int> &results, int a, int b, int c, int d, float res){
    
    float outX = 0.0f;
    float outY = 0.0f;
    
    float count = 1.0f;
    
    float thrsh = 10;
    
    for (int x = 0; x < resolutionX; x++){
        for (int y = 0; y < resolutionY; y++){
            
//            if ( (abs(lookup[x][y][0] - a) +
//                abs(lookup[x][y][1] - b) +
//                abs(lookup[x][y][2] - c) +
//                abs(lookup[x][y][3] - d)) < 4*thrsh ){
            
            if (abs(lookup[x][y][0] - a) < thrsh &&
                 abs(lookup[x][y][1] - b) < thrsh &&
                 abs(lookup[x][y][2] - c) < thrsh &&
                 abs(lookup[x][y][3] - d) < thrsh ){
                
                outX += x;
                outY += y;
            
                count += 1.0f;
                
            }
        }
    }
    
    outX /= count;
    outY /= count;
    
    results.push_back(outX);
    results.push_back(outY);
    
}



CINDER_APP_BASIC( AudioAnalysisOSCApp, RendererGl )

