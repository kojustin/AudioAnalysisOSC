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
    void checkLookup(std::vector<int> &results, int a, int b, int c, int d , int res);
    
    // Audio Variables ////////////////////////////////////////////
    
	audio::Input mInput;
    
	audio::PcmBuffer32fRef mPcmBuffer;
    
    Wave waves [4];
    
    Boolean live;
    Boolean pauseEnabled;
    Boolean delay;
    
    uint16_t  channels = 0;
    
    int something = 2;
    
    const static int resolutionX = 100;
    const static int resolutionY = 100;
    float lookup [resolutionX][resolutionY][4];
    
    // Font Setup ///////////////////////////////////////////////////
    
    Font				mFont;
	gl::TextureFontRef	mTextureFont;
    
    // Graphics Setup ///////////////////////////////////////////////
    
    std::vector<Tap> taps;
    float fade = 0.0f;
    float xScale = 1.0f;
    float yScale = 1.0f;
    
    
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
    pauseEnabled = false;
    
    
    // generate lookup table
    
    for (int x = 0; x < 100; x++){
        for (int y = 0; y < 100; y++){
            
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
            
            for (int i = 0; i < 4; i++){
                lookup[x][y][i] -= lookup[x][y][s];
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
    
//    // this is SO wrong but gives quick dirty results near center
//    float differenceX = (waves[0].startIndex - waves[1].startIndex)*xScale;
//    float differenceY = (waves[2].startIndex - waves[3].startIndex)*yScale;
//    Vec2f pos = Vec2f(getWindowWidth() * 0.5f + differenceX, getWindowHeight() * 0.5f + differenceY);
    
    // arranges time data and finds pos in lookup table
    // the time values must be scaled to the resolution of the lookup table
    // the hypoteneuse of the real grid is scaled to the hypoteneuse of the lookup grid
    
    float scale = sqrt(resolutionX*resolutionX + resolutionY*resolutionY )/sqrt(xScale*xScale + yScale*yScale);
    int a = waves[0].relativeStart* scale;
    int b = waves[1].relativeStart* scale;
    int c = waves[2].relativeStart* scale;
    int d = waves[3].relativeStart* scale;
    
    std::vector<int> posVals;
    
    checkLookup(posVals, a, b, c, d, resolutionX); // uses the largest resolution (default x) for distance measurement
    
    Vec2f pos = Vec2f( (float)posVals[0], (float)posVals[1]);
    
    if (live){
        
        int peakCount = 0;
        
        // check to see if any of the waves have peaked
        
        for (int i = 0; i < channels; i++){
            
            if (waves[i].peaked) peakCount ++;
            
        }
        
        if (peakCount > 0){
            live = false;
            
            // create new tap
            
            taps.push_back( Tap( pos, abs(waves[s].max) ));
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
    
    // find and draw the center offset
    
    gl::color(0.0f, 0.5f, 0.6f);
    
    float differenceX = (waves[0].startIndex - waves[1].startIndex)*xScale;
    float differenceY = (waves[2].startIndex - waves[3].startIndex)*yScale;
    Vec2f triangulate = Vec2f(getWindowWidth() * 0.5f + differenceX, getWindowHeight() * 0.5f + differenceY);
    gl::drawSolidEllipse(triangulate, 10, 10);
    
    displayText();
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
        xScale = getWindowWidth()*0.5/abs(waves[0].startIndex - waves[1].startIndex);
    }
    
    if (e.getChar() == 'y') {
        yScale = getWindowHeight()*0.5/abs(waves[2].startIndex - waves[3].startIndex);
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
    
    
    glPopMatrix();
    
}

void AudioAnalysisOSCApp::checkLookup(std::vector<int> &results, int a, int b, int c, int d, int res){
    
    for (int x = 0; x < 100; x++){
        for (int y = 0; y < 100; y++){
            
            if (lookup[x][y][0] - a < res*0.5 &&
                lookup[x][y][1] - b < res*0.5 &&
                lookup[x][y][2] - c < res*0.5 &&
                lookup[x][y][3] - d < res*0.5){
                
                results.push_back(x);
                results.push_back(y);
                
            }
        }
    }
}



CINDER_APP_BASIC( AudioAnalysisOSCApp, RendererGl )

