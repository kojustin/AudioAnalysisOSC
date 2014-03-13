//
//  Wave.cpp
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 12/03/2014.
//
//

#include "Wave.h"

//#include "cinder/Cinder.h"
//#include "cinder/app/AppBasic.h"
//#include "cinder/audio/FftProcessor.h"
//typedef ci::app::AppBasic AppBase;
//
//#include "cinder/audio/Input.h"
//#include <iostream>
//#include <vector>

using namespace ci;

Wave::Wave(){
    //blank contructor....not really sure why
}

Wave::Wave( cinder::audio::Input mInput, cinder::audio::ChannelIdentifier _channel, float _amp){
    
    channel = _channel;
    
    bandCount = 512;
    
    pcmBuffer = mInput.getPcmBuffer();
    channelBuffer = pcmBuffer->getChannelData( channel );
    mFftDataRef = audio::calculateFft( channelBuffer, bandCount );
    
	delay = false;
    tDelay = 0.0f;
    
    startIndex = 0;
    startPt = * new Vec2f(0.0f, 0.0f);
    
    max = 0.0f;
    amp = _amp;
    
    peaked = false;
    
}

void Wave::update( cinder::audio::Input mInput , uint32_t bufferSamples){
    
    
    // updates current (local) pcm buffer state from argument
    
    pcmBuffer = mInput.getPcmBuffer();
    
    channelBuffer = pcmBuffer->getChannelData( channel );
    
    // updates fft information for the wave
    
    mFftDataRef = cinder::audio::calculateFft( channelBuffer, bandCount );
    
    
    // moved functions from draw
    
    // clear the polyline
    line = *new PolyLine2f();
    
    // buffer samples is 2048 for the focusrite composite device, this is now recieved through argument
    
    int displaySize = cinder::app::getWindowWidth();
    int endIdx = bufferSamples;
    
    //only draw the last 2048(changeable) samples or less
    int32_t startIdx = ( endIdx - 2048 );
    startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
    
    float scale = displaySize / (float)( endIdx - startIdx );
    
    gl::color( Color( 0.0f, 0.8f, 1.0f ) );
    
    max = 0.0f;
    
    for( uint32_t i = startIdx, c = 0; i < endIdx; i++, c++ ) {
        
        float y = ( ( channelBuffer->mData[i] ) * amp  );
        
        if (abs(y) > abs(max)) max = y;
        
        line.push_back( Vec2f( ( c * scale ), y ) );
        
        // find start point of transient
        // and record index at start point
        
        if (i > 10 && delay == false){
            if (abs(channelBuffer->mData[i]*amp) > 2 && abs(channelBuffer->mData[i-10]*amp) < 2 && abs(channelBuffer->mData[i-5]*amp) < 2){
                
                gl::color(1.0f, 1.0f, 1.0f);
                
                startPt.x = c*scale;
                startPt.y = channelBuffer->mData[i]*amp;
                
                startIndex = c;
                
                delay = true;
                
            }
        }
    }
    
    // delay control is now in draw so it definitely updates every frame
    
    if (abs(max) > 20.0f){
        peaked = true;
    }else{
        peaked = false;
    }
    
}

void Wave::drawWave(Boolean * live){
    
    
    if( ! pcmBuffer ) {
        gl::drawSolidEllipse(Vec2f(50.0f, 50.0f), 10.0f, 10.0f);
        return;
    }
    
    // timer for transient stops
    
    if (delay) tDelay += 0.1f;
    if (tDelay > 5.0f){
        delay = false;
        tDelay = 0.0f;
    }
    
    int displaySize = cinder::app::getWindowWidth();
    
    // draws red maximum volume line on a single channel
    
    gl::color( Color( 1.0f, 0.0f, 0.0f ) );
    Vec2f st = *new Vec2f(0.0f, max);
    Vec2f end = *new Vec2f(displaySize, max);
    gl::drawLine(st, end);
    
    // able to change the program state with a pointer
    if (tDelay > 4.5f) *live = true;
    
    // set blue colour of line and draw it
    
    gl::color( Color( 0.0f, 0.8f, 1.0f ) );
    
    gl::draw( line );
    
    // set colour white and draw start point
    
    if (* live == false){
    gl::color(1.0f, 1.0f, 1.0f);
    
    gl::drawSolidEllipse(startPt, 3.0f, 3.0f);
    }
    
}

void Wave::drawFft( float height){
    
    if( ! pcmBuffer ) {
        return;
    }
    
    float ht = 500.0f;
    float bottom = 0.0f;
    
    if( ! mFftDataRef ) {
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