#include "cinder/Cinder.h"


#include "cinder/app/AppBasic.h"
#include "cinder/audio/FftProcessor.h"
typedef ci::app::AppBasic AppBase;

#include "cinder/audio/Input.h"
#include <iostream>
#include <vector>

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
	std::shared_ptr<float> mFftDataRefFL;
    std::shared_ptr<float> mFftDataRefFR;
    std::shared_ptr<float> mFftDataRefBL;
    std::shared_ptr<float> mFftDataRefBR;
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
    
    mPcmBuffer = mInput.getPcmBuffer();
    waves[0] = *new Wave( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ) );
    waves[1] = *new Wave( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_RIGHT ) );
    //    waves[2] = *new Wave( mPcmBuffer->getChannelData( audio::CHANNEL_BACK_RIGHT ) );
    //    waves[3] = *new Wave( mPcmBuffer->getChannelData( audio::CHANNEL_BACK_RIGHT ) );
    
    
	
}

void AudioAnalysisOSCApp::update()
{
    int peakCount = 0;
    
    if (live){
        for (int i = 0; i < channels; i++){
            waves[i].update(mInput.getPcmBuffer());
            
            // counts if any of the waves have gone over the threshold
            if (waves[i].peaked) peakCount++;
        }
        
        if (peakCount > 0) {
            live = false;
        }else{
            live = true;
        }
    }
    
    
}

void AudioAnalysisOSCApp::draw()
{
    
    float waveFormHeight = 100.0;
    
    
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
    
    glPushMatrix();
    glTranslatef(0.0f, getWindowHeight()*0.5f, 0.0f);
    
    for (int i = 0; i < channels; i++){
        waves[i].drawWave( waveFormHeight );
        waves[i].drawFft(500.0f);
    }
    
    glPopMatrix();
}


void AudioAnalysisOSCApp::keyDown(KeyEvent e){
    //    if (e.getChar() == 's') {
    //        mInput.start();
    //    }
    //    if (e.getChar() == 'e') {
    //        mInput.stop();
    //    }
    
    if (e.getChar() == 's') {
        live = true;
    }
}

CINDER_APP_BASIC( AudioAnalysisOSCApp, RendererGl )

