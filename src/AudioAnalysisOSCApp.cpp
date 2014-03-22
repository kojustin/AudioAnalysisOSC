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
//#include <vector>

#include "Wave.h"
#include "Net.h"

using namespace ci;
using namespace ci::app;

class AudioAnalysisOSCApp : public AppBase {
public:
    
    void prepareSettings( Settings *settings);
	void setup();
	void update();
	void draw();
    void keyDown(KeyEvent e);
    
    void teach( vector<float> &inputVals, vector<float> &targetVals);
    void respond( vector<float> &results );
    void displayTags();
    
    // Audio Variables ////////////////////////////////////////////
    
	audio::Input mInput;
    
	audio::PcmBuffer32fRef mPcmBuffer;
    
    Wave waves [4];
    
    Boolean live;
    Boolean pauseEnabled;
    Boolean delay;

    uint16_t  channels = 0;
    
    // Neural Net Variables ////////////////////////////////////////
    
    Net myNet;
    
    vector<vector<float>> trainingSet;
    int trainingId;
    
    vector<unsigned> topology;
    
    vector<float> inputVals;
    
    vector<float> targetVals;
    
    vector<float> resultVals;
    
    Boolean teaching;
    
    // Font Setup ///////////////////////////////////////////////////
    
    Font				mFont;
	gl::TextureFontRef	mTextureFont;
    
    
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
    //    waves[2] = Wave( mPcmBuffer, audio::CHANNEL_BACK_LEFT );
    //    waves[3] = Wave( mPcmBuffer, audio::CHANNEL_BACK_RIGHT );
    
    live = true;
    pauseEnabled = false;
    
    
    
    // Neural Net Setup ////////////////////////////////////////////////////////////
    
    // there will be 9 inputs to the network
    // the 4 relative positions of the transients
    // the 4 max volumes
    // and the average frequency
    
    // the 3 outputs are
    // u position
    // v position
    // gesture type (positive one, negative the other)
    
    topology.push_back(2);
    topology.push_back(5);
//    topology.push_back(5);
    topology.push_back(2);
                       
    myNet = *new Net(topology);
    
    
    targetVals.push_back(1.0f);
    targetVals.push_back(0.0f);
    trainingSet.push_back(targetVals);
    targetVals.clear();
    
    targetVals.push_back(0.0f);
    targetVals.push_back(1.0f);
    trainingSet.push_back(targetVals);
    targetVals.clear();

    
    trainingId = 0;
    
    
    teaching = true;
    
    
    
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
    
    // sets inputVals of the network (currently for 2 channels)
    
    inputVals.push_back(waves[0].relativeStart);
    inputVals.push_back(waves[1].relativeStart);
//    inputVals.push_back(abs(waves[0].max));
//    inputVals.push_back(abs(waves[1].max));
//    inputVals.push_back(waves[0].attack);
//    inputVals.push_back(waves[1].attack);
    
    
    if (live){
        
        int peakCount = 0;
        
        for (int i = 0; i < channels; i++){
            
            // pauses if any of the waves have gone over the threshold and either learn or respond
            if (waves[i].peaked) peakCount ++;

        }
        
        if (peakCount > 0){
            live = false;
            
            // make the network respond
            
            myNet.feedForward(inputVals);
            
            if (teaching) {
                teach( inputVals, trainingSet[trainingId]);
            } else {
                respond( resultVals );
            }
        }
    }
    
    inputVals.clear();
    
}

void AudioAnalysisOSCApp::draw()
{
    
    
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
    
    myNet.displayNet();
    
    glPushMatrix();
    glTranslatef(0.0f, getWindowHeight()/channels - 100.0f, 0.0f);
    
    // update the waves
    
    for (int i = 0; i < channels; i++){
        
        waves[i].drawWave(& live);
        waves[i].drawFft(2000.0f);
        
        glTranslatef(0.0f, 200.0f, 0.0f);
    }
    
    glPopMatrix();
    
    // find and draw the center offset
    
    float difference = waves[0].startIndex - waves[1].startIndex;
    Vec2f triangulate = Vec2f(getWindowWidth() * 0.5f + difference*(getWindowWidth() * 0.5f)/140.0f, getWindowHeight() * 0.5f);
    gl::drawSolidEllipse(triangulate, 10, 10);
    
    displayTags();
}


void AudioAnalysisOSCApp::keyDown(KeyEvent e){
    
    if (e.getChar() == 's') {
        live = true;
    }
    
    if (e.getChar() == 'q') {
        teaching = !teaching;
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
    
    if (e.getChar() == '1') {
        trainingId = 0;;
    }
    if (e.getChar() == '2') {
        trainingId = 1;
    }
    if (e.getChar() == '3') {
        trainingId = 2;
    }
    
    if (e.getChar() == 'f') {
        console() << "freq: " << waves[0].aveFreq << std::endl;
        console() << "max: " << waves[0].max << std::endl;
        console() << "start: " << waves[0].startIndex << std::endl;
        console() << "attack: " << waves[0].attack << std::endl;
    }
    
}

void AudioAnalysisOSCApp::teach( vector<float> &inputVals, vector<float> &targetVals){
    
    myNet.getResults(resultVals); // this function is just to test, if its necessary add a proper argument to the resultVals
    myNet.backProp(targetVals);
    
}

void AudioAnalysisOSCApp::respond( vector<float> &results){
    
    myNet.getResults(results);
    
}

void AudioAnalysisOSCApp::displayTags(){
    
    glPushMatrix();
    glTranslatef(110.0f, 60.0f, 0.0f);
    
    gl::color( Color::white() );
    
    // input layer
    
	mTextureFont->drawString( toString( waves[0].relativeStart ) + " Relative Delay L", Vec2f( 0, 0 ) );
    mTextureFont->drawString( toString( waves[1].relativeStart ) + " Relative Delay R", Vec2f( 0, 50 ) );
    mTextureFont->drawString( toString( floor(abs(waves[0].max)) ) + " Max Volume L", Vec2f( 0, 100 ) );
    mTextureFont->drawString( toString( floor(abs(waves[1].max)) ) + " Max Volume R", Vec2f( 0, 150 ) );
    mTextureFont->drawString( toString( floor(waves[0].attack) ) + " Attack L", Vec2f( 0, 200 ) );
    mTextureFont->drawString( toString( floor(waves[1].attack) ) + " Attack R", Vec2f( 0, 250 ) );
    
    // output layer
    
    glTranslatef(700.0f, 0.0f, 0.0f);
    
    if (resultVals.size() > 0){
    mTextureFont->drawString( toString( resultVals[0] ) + " mm", Vec2f( 0, 0 ) );
//    mTextureFont->drawString( toString( resultVals[1] ) + " Type", Vec2f( 0, 50 ) );
    }
    
    if (pauseEnabled) mTextureFont->drawString( "Pausing Enabled", Vec2f( 150.0f, 0.0f ) );
    if (pauseEnabled) mTextureFont->drawString( toString(Wave::delayThresh / 3.0f) + "sec delayTime", Vec2f( 150.0f, 40.0f ) );
    
    
    glPopMatrix();
    
}

CINDER_APP_BASIC( AudioAnalysisOSCApp, RendererGl )

