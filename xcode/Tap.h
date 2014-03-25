//
//  Tap.h
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 23/03/2014.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Channel.h"
#include "cinder/app/AppBasic.h"
#include "cinder/audio/FftProcessor.h"
typedef ci::app::AppBasic AppBase;
#include <iostream>

using namespace ci;

class Tap{
    
public:
    Tap();
    Tap( Vec2f _pos, float _size );
    
    void run();
    
    Vec2f pos;
    float size;
    float t;
    
private:
   
};