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
    Wave( cinder::audio::Input mInput, cinder::audio::ChannelIdentifier _channel);
         
	void update( cinder::audio::Input mInput);
	void drawWave(uint32_t bufferSamples, float amp);
    void drawFft( float height);
	
    // for audio stream
	cinder::audio::PcmBuffer32fRef pcmBuffer;
    cinder::audio::Buffer32fRef  channelBuffer;
    cinder::audio::ChannelIdentifier channel;
    
    // for fft
    std::shared_ptr<float> mFftDataRef;
    uint16_t bandCount;
    
    // for start index
	Boolean delay;
    float tDelay;
    int startIndex;
    
    Boolean peaked;

};
