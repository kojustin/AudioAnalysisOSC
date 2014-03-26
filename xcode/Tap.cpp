//
//  Tap.cpp
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 23/03/2014.
//
//

#include "Tap.h"

using namespace ci;

Tap::Tap(){
    
}

Tap::Tap( Vec2f _pos, float _size ){
    
    pos = _pos;
    size = _size;
    
    t = 0.0f;
    
    mode = 0;
    
}

void Tap::run(){
    
    if (mode == 0){
        gl::color( 1.0f, 1.0f, 1.0f, 1.0f-t );
    }else{
        gl::color( 1.0f, 0.0f, 0.0f);
    }
    
    gl::lineWidth(size*0.25f - size*0.25f*t);
    gl::drawStrokedCircle( pos, 10 + size*t );
    
    t += 0.01f;
    
    gl::lineWidth(1);
}