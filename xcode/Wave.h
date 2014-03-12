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
    Wave( cinder::audio::PcmBuffer32fRef _mPcmBuffer, cinder::audio::Buffer32fRef _channelBuffer);
         
	void update();
	void drawWave();
    void drawFft();
	
	cinder::audio::PcmBuffer32fRef mPcmBuffer;
    cinder::audio::Buffer32fRef channelBuffer;
    
	Boolean delay;
    float tDelay;
    
    int startIndex;

};
