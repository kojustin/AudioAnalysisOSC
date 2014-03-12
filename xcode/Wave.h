//
//  Wave.h
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 12/03/2014.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Channel.h"
#include "cinder/app/AppBasic.h"
#include "cinder/audio/FftProcessor.h"
typedef ci::app::AppBasic AppBase;

#include "cinder/audio/Input.h"
#include <iostream>
#include <vector>


class Wave {
public:
	Wave();
    Wave( cinder::audio::Buffer32fRef _channelBuffer);
         
	void update( cinder::audio::PcmBuffer32fRef mPcm);
	void drawWave( float amp);
    void drawFft( float height);
	
    // for audio stream
	cinder::audio::PcmBuffer32fRef mPcmBuffer;
    cinder::audio::Buffer32fRef channelBuffer;
    
    // for fft
    std::shared_ptr<float> mFftDataRef;
    uint16_t bandCount;
    
    // for start index
	Boolean delay;
    float tDelay;
    int startIndex;
    
    Boolean peaked;

};
