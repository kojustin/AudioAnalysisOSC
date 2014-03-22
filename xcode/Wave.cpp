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

float Wave::delayThresh = 3.0f;

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
    relativeStart = 0.0f;
    startPt = * new Vec2f(0.0f, 0.0f);
    
    max = 0.0f;
    amp = _amp;
    aveFreq = 0.0f;
    attack = 0.0f;
    
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
        
        if (i > 50 && delay == false){
            
            int count = 0;
            
            for (int j = 5; j < 50; j+=5){
                if (abs(channelBuffer->mData[i-j]*amp) > 2) count++;
            }
            
            if (abs(channelBuffer->mData[i]*amp) > 2 && count < 1){
                
                gl::color(1.0f, 1.0f, 1.0f);
                
                // -5 to shift the start point back a little
                
                startPt.x = (c-5)*scale;
                startPt.y = channelBuffer->mData[i-5]*amp;
                
                startIndex = c-5;
                
                delay = true;
                
                // gather attack gradient information
                
                if (i < bufferSamples - 20){
                    attack = abs( channelBuffer->mData[i]*amp - channelBuffer->mData[i+19]*amp);
                }
                
            }
        }
    }
    
    // delay control is now in draw so it definitely updates every frame
    
    if (abs(max) > 10.0f){
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
    if (tDelay > delayThresh){
        delay = false;
        tDelay = 0.0f;
        *live = true;
    }
    
    int displaySize = cinder::app::getWindowWidth();
    
    // draws red maximum volume line on a single channel
    
    gl::color( Color( 1.0f, 0.0f, 0.0f ) );
    Vec2f st = *new Vec2f(0.0f, max);
    Vec2f end = *new Vec2f(displaySize, max);
    gl::drawLine(st, end);
    
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

    float bottom = 0.0f;
    
    if( ! mFftDataRef ) {
        return;
    }
    
    float * fftBuffer = mFftDataRef.get();
    
    float maxFreq = 0.0f;
    int freqIdx = 0;
    
    for( int i = 0; i < ( bandCount ); i++ ) {
        float barY = fftBuffer[i] / bandCount * height;
        
        if (barY > maxFreq){
            maxFreq = barY;
            freqIdx = i;
        }
        
        glBegin( GL_QUADS );
        glColor3f( 0.0f, 100.0f, 255.0f );
        glVertex2f( i * 5, bottom );
        glVertex2f( i * 5 + 4, bottom );
        glColor3f( 0.0f, 255.0f, 0.0f );
        glVertex2f( i * 5 + 4, bottom - barY );
        glVertex2f( i * 5, bottom - barY );
        glEnd();
        
    }

    if (peaked){
    aveFreq = freqIdx;
    } else {
        aveFreq = 0;
    }
    
}