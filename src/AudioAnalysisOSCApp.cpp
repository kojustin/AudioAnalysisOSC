#include "cinder/Cinder.h"

#include "cinder/app/AppBasic.h"
#include "cinder/audio/FftProcessor.h"
typedef ci::app::AppBasic AppBase;

//#include "cinder/audio/Input.h"
//#include <iostream>
//#include <vector>

#include "Wave.h"

using namespace ci;
using namespace ci::app;

class AudioAnalysisOSCApp : public AppBase {
public:
    void prepareSettings( Settings *settings);
	void setup();
	void update();
	void draw();
    
    void keyDown(KeyEvent e);
    
	audio::Input mInput;
    
	audio::PcmBuffer32fRef mPcmBuffer;
    
    Wave waves [4];
    
    Boolean live;
    Boolean delay;
    float tDelay;
    uint16_t  channels = 0;
    
};

void AudioAnalysisOSCApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 1280, 800 );
    //settings->setFrameRate( 60.0f );
}

void AudioAnalysisOSCApp::setup()
{
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

CINDER_APP_BASIC( AudioAnalysisOSCApp, RendererGl )

