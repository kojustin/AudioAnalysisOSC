#include "cinder/Cinder.h"


#include "cinder/app/AppBasic.h"
#include "cinder/audio/FftProcessor.h"
typedef ci::app::AppBasic AppBase;

#include "cinder/audio/Input.h"
#include <iostream>
#include <vector>

using namespace ci;
using namespace ci::app;

class AudioAnalysisOSCApp : public AppBase {
public:
    void prepareSettings( Settings *settings);
	void setup();
	void update();
	void draw();
	void drawWaveForms( float height );
    
	void drawFft( std::shared_ptr<float> mFftDataRef );
    void keyDown(KeyEvent e);
    
	
	audio::Input mInput;
	std::shared_ptr<float> mFftDataRefFL;
    std::shared_ptr<float> mFftDataRefFR;
    std::shared_ptr<float> mFftDataRefBL;
    std::shared_ptr<float> mFftDataRefBR;
	audio::PcmBuffer32fRef mPcmBuffer;
    
    Boolean live;
    Boolean delay;
    double tDelay;
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
    
    // sets the live to true to start
    live = true;
    delay = false;
    tDelay = 0;
	
}

void AudioAnalysisOSCApp::update()
{
    
    if (live){
        mPcmBuffer = mInput.getPcmBuffer();
        if( ! mPcmBuffer ) {
            return;
        }
        
        uint16_t bandCount = 512;
        //presently FFT only works on OS X, not iOS
        mFftDataRefFL = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );
        mFftDataRefFR = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_RIGHT), bandCount );
        
        //            mFftDataRefBL = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_BACK_LEFT), bandCount );
        //            mFftDataRefBR = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_BACK_RIGHT), bandCount );
        
    }
}

void AudioAnalysisOSCApp::draw()
{
    
    float waveFormHeight = 100.0;
    
    
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
    
    glPushMatrix();
    glTranslatef(0.0f, getWindowHeight()*0.5f, 0.0f);
    
    
    drawWaveForms( waveFormHeight );
    
    glPushMatrix();
    glTranslatef(0.0f, -300.0f, 0.0f);
    drawFft(mFftDataRefFL);
    glTranslatef(0.0f, 150.0f, 0.0f);
    drawFft(mFftDataRefFR);
    //    glTranslatef(0.0f, 150.0f, 0.0f);
    //    drawFft(mFftDataRefBL);
    //    glTranslatef(0.0f, 150.0f, 0.0f);
    //    drawFft(mFftDataRefBR);
    glPopMatrix();
    
    glPopMatrix();
}

void AudioAnalysisOSCApp::drawWaveForms( float height )
{
    if( ! mPcmBuffer ) {
        return;
    }
    
    // buffer samples is 2048 for the focusrite composite device
    
    uint32_t bufferSamples = mPcmBuffer->getSampleCount();
    //console() << bufferSamples << std::endl;
    
    audio::Buffer32fRef fLeftBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT );
    audio::Buffer32fRef fRightBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_RIGHT );
    
    //    audio::Buffer32fRef bLeftBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_BACK_LEFT );
    //    audio::Buffer32fRef bRightBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_BACK_RIGHT );
    
    
    int displaySize = getWindowWidth();
    int endIdx = bufferSamples;
    
    //only draw the last 2000(changeable) samples or less
    int32_t startIdx = ( endIdx - 2000 );
    startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
    
    float scale = displaySize / (float)( endIdx - startIdx );
    
    PolyLine<Vec2f>	linefL;
    PolyLine<Vec2f>	linefR;
    PolyLine<Vec2f>	linebL;
    PolyLine<Vec2f>	linebR;
    
    gl::color( Color( 0.0f, 0.8f, 1.0f ) );
    
    float maxfL = 0;
    float maxfR = 0;
    //    float maxbL = 0;
    //    float maxbR = 0;
    
    for( uint32_t i = startIdx, c = 0; i < endIdx; i++, c++ ) {
        
        // it seems that zero signal returns a -1 in the mdata so an extra displacement is needed
        // 100 or - 200 is used to coalesce the transient direction
        
        float yfL = ( ( fLeftBuffer->mData[i] ) * 100  );
        float yfR = ( ( fRightBuffer->mData[i] ) * - 100 );
        //        float ybL = ( ( bLeftBuffer->mData[i] ) * 100 );
        //        float ybR = ( ( bRightBuffer->mData[i] ) * 100 );
        
        
        if (abs(yfL) > abs(maxfL)) maxfL = yfL;
        if (abs(yfR) > abs(maxfR)) maxfR = yfR;
        //        if (abs(ybL) > abs(maxbL)) maxbR = ybL;
        //        if (abs(ybR) > abs(maxbR)) maxbL = ybR;
        
        
        
        linefL.push_back( Vec2f( ( c * scale ), yfL ) );
        linefR.push_back( Vec2f( ( c * scale ), yfR ) );
        //        linebL.push_back( Vec2f( ( c * scale ), ybL ) );
        //        linebR.push_back( Vec2f( ( c * scale ), ybR ) );
        
        // draw ellipse at start point of transient
        
        if (i > 10 && delay == false){
            if (abs(fLeftBuffer->mData[i]*100) > 2 && abs(fLeftBuffer->mData[i-10]*100) < 2){
                
                gl::color(c*0.01, c*0.01, c*0.01);
                
                glPushMatrix();
                glTranslatef(0.0f, -300.0f, 0.0f);
                const Vec2f start = *new Vec2f(c*scale, fLeftBuffer->mData[i]*100);
                gl::drawSolidEllipse(start, 3.0f, 3.0f);
                
                glPopMatrix();
                
                delay = true;
                
            }
        }
    }
    
    if (delay) tDelay += 0.1;
    if (tDelay > 10) delay = false;
    
    if (abs(maxfL) > 20 || abs(maxfR) > 20) live = false;
    
    // draws red maximum volume line on a single channel
    
    gl::color( Color( 1.0f, 0.0f, 0.0f ) );
    Vec2f st = *new Vec2f(0.0f, maxfL - 300.0f);
    Vec2f end = *new Vec2f(getWindowWidth(), maxfL -300.0f);
    gl::drawLine(st, end);
    
    
    // set blue colour of line
    
    gl::color( Color( 0.0f, 0.8f, 1.0f ) );
    
    glPushMatrix();
    glTranslatef(0.0f, -300.0f, 0.0f);
    gl::draw( linefL );
    glTranslatef(0.0f, 150.0f, 0.0f);
    gl::draw( linefR );
    glTranslatef(0.0f, 150.0f, 0.0f);
    gl::draw( linebL );
    glTranslatef(0.0f, 150.0f, 0.0f);
    gl::draw( linebR );
    glPopMatrix();
    
    
}

void AudioAnalysisOSCApp::drawFft( std::shared_ptr<float> mFftDataRef )
{
    if( ! mPcmBuffer ) {
        return;
    }
    
    uint16_t bandCount = 512;
    float ht = 1000.0f;
    float bottom = 0.0f;
    
    if( ! mFftDataRefFL ) {
        return;
    }
    
    float * fftBuffer = mFftDataRef.get();
    
    for( int i = 0; i < ( bandCount ); i++ ) {
        float barY = fftBuffer[i] / bandCount * ht;
        glBegin( GL_QUADS );
        glColor3f( 255.0f, 255.0f, 0.0f );
        glVertex2f( i * 3, bottom );
        glVertex2f( i * 3 + 1, bottom );
        glColor3f( 0.0f, 255.0f, 0.0f );
        glVertex2f( i * 3 + 1, bottom - barY );
        glVertex2f( i * 3, bottom - barY );
        glEnd();
    }
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

