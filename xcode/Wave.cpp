//
//  Wave.cpp
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 12/03/2014.
//
//

#include "Wave.h"

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"
#include "cinder/audio/FftProcessor.h"
typedef ci::app::AppBasic AppBase;

#include "cinder/audio/Input.h"
#include <iostream>
#include <vector>

using namespace ci;

Wave::Wave(){
    //blank contructor....not really sure why
}

Wave::Wave( cinder::audio::Input mInput, cinder::audio::ChannelIdentifier _channel){
    
    // main pcm buffer
    
    channel = _channel;
    
    bandCount = 256;
    
    pcmBuffer = mInput.getPcmBuffer();
    channelBuffer = pcmBuffer->getChannelData( channel );
    mFftDataRef = audio::calculateFft( channelBuffer, bandCount );
    
	delay = false;
    tDelay = 0.0f;
    
    startIndex = 0;
    
    peaked = false;
    
}

void Wave::update( cinder::audio::Input mInput ){
    
    
    // updates current pcm buffer state from argument
    
    pcmBuffer = mInput.getPcmBuffer();
    
    channelBuffer = pcmBuffer->getChannelData( channel );
    
    // updates fft information for the wave
    
    mFftDataRef = cinder::audio::calculateFft( channelBuffer, bandCount );
    
}

void Wave::drawWave( uint32_t bufferSamples, float amp){
    
    
    if( ! pcmBuffer ) {
        gl::drawSolidEllipse(Vec2f(50.0f, 50.0f), 10.0f, 10.0f);
        return;
    }
    
    // buffer samples is 2048 for the focusrite composite device
    
    channelBuffer = pcmBuffer->getChannelData( channel );
    
    int displaySize = cinder::app::getWindowWidth();
    int endIdx = bufferSamples;
    
    //only draw the last 2000(changeable) samples or less
    int32_t startIdx = ( endIdx - 2048 );
    startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
    
    float scale = displaySize / (float)( endIdx - startIdx );
    
    PolyLine<Vec2f>	line;
    
    gl::color( Color( 0.0f, 0.8f, 1.0f ) );
    
    float max = 0.0f;
    
    for( uint32_t i = startIdx, c = 0; i < endIdx; i++, c++ ) {
        
        // it seems that zero signal returns a -1 in the mdata so an extra displacement is needed
        // 100 or - 200 is used to coalesce the transient direction
        
        float y = ( (float)( channelBuffer->mData[i] ) * amp  );
        
        if (abs(y) > abs(max)) max = y;
        
        line.push_back( Vec2f( ( c * scale ), y ) );
        
        
        // draw ellipse at start point of transient
        
        if (i > 10 && delay == false){
            if (abs(channelBuffer->mData[i]*amp) > 2 && abs(channelBuffer->mData[i-10]*amp) < 2){
                
                gl::color(1.0f, 1.0f, 1.0f);
                
                const Vec2f start = *new Vec2f(c*scale, channelBuffer->mData[i]*100);
                gl::drawSolidEllipse(start, 3.0f, 3.0f);
                
                delay = true;
                
            }
        }
    }
    
    if (delay) tDelay += 0.1f;
    if (tDelay > 5.0f) delay = false;
    
    if (abs(max) > 10.0f){
        peaked = true;
    }else{
        peaked = false;
    }
    
    
    // draws red maximum volume line on a single channel
    
    gl::color( Color( 1.0f, 0.0f, 0.0f ) );
    Vec2f st = *new Vec2f(0.0f, max);
    Vec2f end = *new Vec2f(displaySize, max);
    gl::drawLine(st, end);
    
    
    // set blue colour of line and draw it
    
    gl::color( Color( 0.0f, 0.8f, 1.0f ) );
    
    gl::draw( line );
    
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