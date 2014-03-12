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

Wave::Wave( cinder::audio::PcmBuffer32fRef _mPcmBuffer, cinder::audio::Buffer32fRef _channelBuffer ){
    
    // main pcm buffer
    
    mPcmBuffer = _mPcmBuffer;
    channelBuffer = channelBuffer;
    
	delay = false;
    tDelay = 0;
    
    startIndex = 0;

}