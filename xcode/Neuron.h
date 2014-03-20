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
    
    double weight;
    double deltaWeight;
    
};


class Neuron{
    
public:
    Neuron(unsigned numOutputs, unsigned myIndex);
    void setOutputVal(double val) { m_outputVal = val; }
    double getOutputVal(void) const { return m_outputVal; }
    void feedForward(const Layer &prevLayer);
    void calcOutputGradients(double targetVal);
    void calcHiddenGradients(const Layer &nextLayer);
    void updateInputWeights( Layer &prevLayer);
    
    // moved from private to be accessed from the Net::display function
    std::vector<Connection> m_outputWeights;
    
private:
    static double eta; // [0.0 ... 1.0] overall training rate
    static double alpha; // [0.0 .. n] multiplier of last weight change, ie learning momentum
    
    // function for returning a random weight (needs the include-<cstdlib>
    static double randomWeight(void){ return rand()/double(RAND_MAX); }
    static double transferFunction(double x);
    static double transferFunctionDerivative(double x);
    double sumDOW(const Layer &nextLayer) const;
    
    double m_outputVal;

    
    unsigned m_myIndex;
    double m_gradient;
    
};

double Neuron::eta = 0.15;
double Neuron::alpha = 0.5;

