#include "cinder/Cinder.h"

#include "cinder/app/AppBasic.h"
#include "cinder/audio/FftProcessor.h"
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
    
    void teach(Net myNet, vector<double> inputVals, vector<double> targetVals);
    void respond(Net myNet, vector<double> results);
    
    // Audio Variables ////////////////////////////////////////////
    
	audio::Input mInput;
    
	audio::PcmBuffer32fRef mPcmBuffer;
    
    Wave waves [4];
    
    Boolean live;
    Boolean delay;

    uint16_t  channels = 0;
    
    // Neural Net Variables ////////////////////////////////////////
    
    
    Net myNet;
    
    vector<float> trainingSet;
    
    vector<unsigned> topology;
    
    vector<double> inputVals;
    
    vector<double> targetVals;
    
    vector<double> resultVals;
    
    
    
    
};

void AudioAnalysisOSCApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 1280, 800 );
    //settings->setFrameRate( 60.0f );
}

void AudioAnalysisOSCApp::setup()
{
    
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
    
    waves[0] = *new Wave( mInput, audio::CHANNEL_FRONT_LEFT , 200.0f);
    waves[1] = *new Wave( mInput, audio::CHANNEL_FRONT_RIGHT , 200.0f);
    //    waves[2] = *new Wave( mPcmBuffer, audio::CHANNEL_BACK_LEFT );
    //    waves[3] = *new Wave( mPcmBuffer, audio::CHANNEL_BACK_RIGHT );
    
    live = true;
    
    
    
    
    // Neural Net Setup ////////////////////////////////////////////////////////////
    
    // there will be 9 inputs to the network
    // the 4 relative positions of the transients
    // the 4 max volumes
    // and the average frequency
    
    // the 3 outputs are
    // u position
    // v position
    // gesture type (positive one, negative the other)
    
    topology.push_back(6);
    topology.push_back(12);
    topology.push_back(3);
                       
    myNet = *new Net(topology);
    
    for (unsigned i = 0; i < trainingSet.size(); i++){
        trainingSet.push_back(i*5.0f);
    }
    
    
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
    if (live){
        for (int i = 0; i < channels; i++){
            
            waves[i].update(mInput, bufferSamples);
            
            // pauses if any of the waves have gone over the threshold
            if (waves[i].peaked){
                live = false;
            }
            
        }
        
    }
    
}

void AudioAnalysisOSCApp::draw()
{
    
    
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
    
    myNet.displayNet();
    
    glPushMatrix();
    glTranslatef(0.0f, getWindowHeight()/channels - 100.0f, 0.0f);
    
    for (int i = 0; i < channels; i++){
        
        waves[i].drawWave(& live);
        waves[i].drawFft(2000.0f);
        
        glTranslatef(0.0f, 200.0f, 0.0f);
    }
    
    glPopMatrix();
    
    float difference = waves[0].startIndex - waves[1].startIndex;
    Vec2f triangulate = * new Vec2f(getWindowWidth() * 0.5f + difference*(getWindowWidth() * 0.5f)/140.0f, getWindowHeight() * 0.5f);
    gl::drawSolidEllipse(triangulate, 10, 10);
}


void AudioAnalysisOSCApp::keyDown(KeyEvent e){
    
    if (e.getChar() == 's') {
        live = true;
    }
    
    if (e.getChar() == 'f') {
        console() << "freq: " << waves[0].aveFreq << std::endl;
        console() << "max: " << waves[0].max << std::endl;
        console() << "start: " << waves[0].startIndex << std::endl;
        console() << "attack: " << waves[0].attack << std::endl;
    }
    
}

void AudioAnalysisOSCApp::teach(Net myNet, vector<double> inputVals, vector<double> targetVals){
    
    myNet.feedForward(inputVals);
    myNet.feedForward(targetVals);
    
}

void AudioAnalysisOSCApp::respond(Net myNet, vector<double> results){
    
    myNet.getResults(results);
    
}

CINDER_APP_BASIC( AudioAnalysisOSCApp, RendererGl )

