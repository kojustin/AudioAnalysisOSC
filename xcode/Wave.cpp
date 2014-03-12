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

Wave::Wave( cinder::audio::Buffer32fRef _channelBuffer ){
    
    // main pcm buffer

    channelBuffer = _channelBuffer;
    
	delay = false;
    tDelay = 0;
    
    startIndex = 0;
    
    bandCount = 256;
    mFftDataRef = 0;
    
    peaked = false;
    
    mFftDataRef = audio::calculateFft( channelBuffer, bandCount );

}

void Wave::update(cinder::audio::PcmBuffer32fRef mPcm){
    
    // should take mInput.getPcmBuffer() as input
    
    // updates current pcm buffer state from argument
    
    mPcmBuffer = mPcm;
    if( ! mPcmBuffer ) {
        return;
    }
    
    // updates fft information for the wave
    
    mFftDataRef = audio::calculateFft( channelBuffer, bandCount );
    
}

void Wave::drawWave( float amp){

    
    gl::drawSolidEllipse(Vec2f(50.0f, 50.0f), 10.0f, 10.0f);
    
    if( ! mPcmBuffer ) {
        return;
    }
    
    // POTENTIAL PROBLEM: mpcmBuffer might not be passed in as a reference, which means it wont be continuously updated
    //                    if you find out what "->" really means this might solve the issue
    
    // buffer samples is 2048 for the focusrite composite device
    
    uint32_t bufferSamples = mPcmBuffer->getSampleCount();
    //console() << bufferSamples << std::endl;
    
    int displaySize = cinder::app::getWindowWidth();
    int endIdx = bufferSamples;
    
    //only draw the last 2000(changeable) samples or less
    int32_t startIdx = ( endIdx - 2000 );
    startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
    
    float scale = displaySize / (float)( endIdx - startIdx );
    
    PolyLine<Vec2f>	line;
    
    gl::color( Color( 0.0f, 0.8f, 1.0f ) );
    
    float max = 0;
    
    for( uint32_t i = startIdx, c = 0; i < endIdx; i++, c++ ) {
        
        // it seems that zero signal returns a -1 in the mdata so an extra displacement is needed
        // 100 or - 200 is used to coalesce the transient direction
        
        float yfL = ( ( channelBuffer->mData[i] ) * amp  );
        
        if (abs(yfL) > abs(max)) max = yfL;
        
        line.push_back( Vec2f( ( c * scale ), yfL ) );

        
        // draw ellipse at start point of transient
        
        if (i > 10 && delay == false){
            if (abs(channelBuffer->mData[i]*amp) > 2 && abs(channelBuffer->mData[i-10]*amp) < 2){
                
                gl::color(c*0.01, c*0.01, c*0.01);

                const Vec2f start = *new Vec2f(c*scale, channelBuffer->mData[i]*100);
                gl::drawSolidEllipse(start, 3.0f, 3.0f);
                
                delay = true;
                
            }
        }
    }
    
    if (delay) tDelay += 0.1f;
    if (tDelay > 5) delay = false;
    
    if (abs(max) > 20 || abs(max) > 20) peaked = true;

    
    // draws red maximum volume line on a single channel
    
    gl::color( Color( 1.0f, 0.0f, 0.0f ) );
    Vec2f st = *new Vec2f(0.0f, max - 300.0f);
    Vec2f end = *new Vec2f(displaySize, max - 300.0f);
    gl::drawLine(st, end);
    
    
    // set blue colour of line
    
    gl::color( Color( 0.0f, 0.8f, 1.0f ) );
    
    gl::draw( line );

}

void Wave::drawFft( float height){
    
    if( ! mPcmBuffer ) {
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