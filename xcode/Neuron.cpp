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
        m_outputWeights.back().weight = randomWeight();
        
    }
    
    m_myIndex = myIndex;
    
}

void Neuron::feedForward( const Layer &prevLayer){
    
    double sum = 0.0;
    
    // Sum the previous layer's outputs ( which are our inputs)
    // Include the bias node from the precious layer.
    
    for (unsigned n = 0; n < prevLayer.size(); n++){
        
        sum += prevLayer[n].getOutputVal() * prevLayer[n].m_outputWeights[m_myIndex].weight;
        
    }
    
    m_outputVal = transferFunction(sum);
    
}

void Neuron::updateInputWeights( Layer &prevLayer){
    
    // the weights to be updated are in the connection container
    // in the neurons in the preceding layer
    
    for (unsigned n = 0; n < prevLayer.size(); n++){
        
        Neuron &neuron = prevLayer[n];
        double oldDeltaWeight = neuron.m_outputWeights[m_myIndex].deltaWeight;
        
        double newDeltaWeight =
            // Individual input, magnified by the gradient and train rate;
            eta
            *neuron.getOutputVal()
            * m_gradient
            // also add the momentum  = a fraction of the previous delta weight
            + alpha // momentum rate
            * oldDeltaWeight;
        
        neuron.m_outputWeights[m_myIndex].deltaWeight = newDeltaWeight;
        neuron.m_outputWeights[m_myIndex].weight += newDeltaWeight;
    }
    
}

double Neuron::sumDOW(const Layer &nextLayer) const{
    
    double sum = 0.0;
    
    // Sum our contributions to the errors at the nodes we feed
    // ( -1 one excludes the bias neuron)
    
    for (unsigned n = 0; n < nextLayer.size() - 1; n++){
        
        sum += m_outputWeights[n].weight * nextLayer[n].m_gradient;
        
    }
    
    return sum;
}


void Neuron::calcHiddenGradients(const Layer &nextLayer){
    
    double dow = sumDOW(nextLayer);
    m_gradient = dow * Neuron::transferFunctionDerivative(m_outputVal);
    
    
}


void Neuron::calcOutputGradients(double targetVal){
    
    double delta = targetVal - m_outputVal;
    m_gradient = delta * Neuron::transferFunctionDerivative(m_outputVal);
    
}

double Neuron::transferFunction(double x){
    
    // tanh - output range [-1.0 ... 1.0]
    
    return tanh(x);
    
}

double Neuron::transferFunctionDerivative( double x ){
    
    return 1.0 - x * x;
    
}