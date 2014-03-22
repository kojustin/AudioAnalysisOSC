//
//  Neuron.cpp
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 14/03/2014.
//
//

#include "Neuron.h"
#include <cmath>


Neuron::Neuron(unsigned numOutputs, unsigned myIndex){
    
    for (unsigned c = 0; c < numOutputs; c++){
        
        m_outputWeights.push_back(Connection());
//        m_outputWeights.back().weight = randomWeight();
        m_outputWeights.back().weight = 0.5f;
        
    }
    
    m_myIndex = myIndex;
    
    eta = 0.5f;
    alpha = 0.4f;
    
}

void Neuron::feedForward( const Layer &prevLayer){
    
    float sum = 0.0f;
    
    // Sum the previous layer's outputs ( which are our inputs)
    // Include the bias node from the previous layer.
    
    for (unsigned n = 0; n < prevLayer.size(); n++){
        
        sum += prevLayer[n].getOutputVal() * prevLayer[n].m_outputWeights[m_myIndex].weight;
        
    }
    
    m_outputVal = Neuron::transferFunction(sum);
    
}

void Neuron::updateInputWeights( Layer &prevLayer){
    
    // the weights to be updated are in the connection container
    // in the neurons in the preceding layer
    
    for (unsigned n = 0; n < prevLayer.size(); n++){
        
        Neuron &neuron = prevLayer[n];
        float oldDeltaWeight = neuron.m_outputWeights[m_myIndex].deltaWeight;
        
//        float newDeltaWeight =
//        // Individual input, magnified by the gradient and train rate;
//        eta
//        *neuron.getOutputVal()
//        * m_gradient
//        // also add the momentum  = a fraction of the previous delta weight
//        + alpha // momentum rate
//        * oldDeltaWeight;
        
        float newDeltaWeight = eta * neuron.getOutputVal() * m_gradient + alpha * oldDeltaWeight;
        
        neuron.m_outputWeights[m_myIndex].deltaWeight = newDeltaWeight;
        neuron.m_outputWeights[m_myIndex].weight += newDeltaWeight;
    }
    
}

float Neuron::sumDOW(const Layer &nextLayer) const{
    
    float sum = 0.0f;
    
    // Sum our contributions to the errors at the nodes we feed
    // ( -1 one excludes the bias neuron)
    
    for (unsigned n = 0; n < nextLayer.size() - 1; n++){
        
        sum += m_outputWeights[n].weight * nextLayer[n].m_gradient;
        
    }
    
    return sum;
}


void Neuron::calcHiddenGradients(const Layer &nextLayer){
    
    float dow = sumDOW(nextLayer);
    m_gradient = dow * Neuron::transferFunctionDerivative(m_outputVal);
    
    
}


void Neuron::calcOutputGradients(float targetVal){
    
    float delta = targetVal - m_outputVal;
    m_gradient = delta * Neuron::transferFunctionDerivative(m_outputVal);
    
}

float Neuron::transferFunction(float x){
    
    // tanh - output range [-1.0 ... 1.0]
    
    // return tanh(x);
    return 1/(1 + exp(-x));
    
}

float Neuron::transferFunctionDerivative( float x ){
    
//    return 1.0f - x * x;
//    return sinh(x)/cosh(x);
    
    float f = 1/(1 + exp(-x));
    
    return f*(1-f);
    
}