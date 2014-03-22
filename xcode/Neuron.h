//
//  Neuron.h
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 14/03/2014.
//
//

#ifndef __AudioAnalysisOSC__Neuron__
#define __AudioAnalysisOSC__Neuron__

#include <iostream>
#include <cstdlib>

#endif /* defined(__AudioAnalysisOSC__Neuron__) */

class Neuron;

typedef std::vector<Neuron> Layer;

// Connection struct holds the weight and change in weight
// needed to counteract the error.

struct Connection{
    
    float weight;
    float deltaWeight;
    
};


class Neuron{
    
public:
    Neuron(unsigned numOutputs, unsigned myIndex);
    void setOutputVal(float val) { m_outputVal = val; }
    float getOutputVal(void) const { return m_outputVal; }
    std::vector<Connection> getOutputWeights() const { return m_outputWeights; };
    void feedForward(const Layer &prevLayer);
    void calcOutputGradients(float targetVal);
    void calcHiddenGradients(const Layer &nextLayer);
    void updateInputWeights( Layer &prevLayer);
    
    // moved from private to be accessed from the Net::display function
    //std::vector<Connection> m_outputWeights;
    
private:
    
    // function for returning a random weight (needs the include-<cstdlib>
    static float randomWeight(void){ return rand()/float(RAND_MAX); }
    static float transferFunction(float x);
    static float transferFunctionDerivative(float x);
    float sumDOW(const Layer &nextLayer) const;
    
    float m_outputVal;

    std::vector<Connection> m_outputWeights;
    
    unsigned m_myIndex;
    float m_gradient;
    
    float eta; // [0.0 ... 1.0] overall training rate
    float alpha; // [0.0 .. n] multiplier of last weight change, ie learning momentum
    
};

