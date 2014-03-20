//
//  Net.cpp
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 14/03/2014.
//
//

#include "Net.h"

using namespace ci;

Net::Net(){
    // blank constructor
}

Net::Net( const vector<unsigned> &topology ){
    
    // creates the net with set layers and neurons
    
    unsigned numLayers = topology.size();
    
    for (unsigned layerNum = 0; layerNum < numLayers; layerNum++){
        
        m_layers.push_back(Layer());
        unsigned numOutputs = layerNum == topology.size()-1 ? 0 : topology[layerNum + 1];
        
        // we have made a new layer, now fill it with neurons and
        // add a bias neuron to the layer
        
        for (unsigned neuronNum = 0; neuronNum <= topology[layerNum]; neuronNum++){
            
            // takes the last layer created and appends a neuron on the end
            m_layers.back().push_back(Neuron(numOutputs, neuronNum));
            
        }
        
    }
    
}

void Net::getResults(vector<double> &resultVals) const{
    
    resultVals.clear();
    
    for (unsigned n = 0; n < m_layers.back().size() - 1; n++){
        resultVals.push_back(m_layers.back()[n].getOutputVal());
    }
    
}

void Net::feedForward(const vector<double> &inputVals){
    
    assert(inputVals.size() == m_layers[0].size() - 1);
    
    // assign (latch) the input values into the input neurons
    
    for (unsigned i = 0; i < inputVals.size(); i++){
        
        m_layers[0][i].setOutputVal(inputVals[i]);
        
    }
    
    // forward propagate
    
    for (unsigned layerNum = 1; layerNum < m_layers.size(); layerNum++){
        
        Layer &prevLayer = m_layers[layerNum - 1];
        
        for (unsigned n = 0; n < m_layers[layerNum].size() - 1; n++){
            
            // the neuron feedForward() is different to the net feedForward()
            // the prevLayer argument
            m_layers[layerNum][n].feedForward(prevLayer);
            
        }
        
    }
    
}


void Net::backProp( const vector<double> &targetVals){
    
    
    // calculate overall net error (RMS of output neuron errors)
    
    Layer &outputLayer = m_layers.back();
    m_error = 0.0;
    
    for (unsigned n = 0; n < outputLayer.size() - 1; n++){
        
        double delta = targetVals[n] - outputLayer[n].getOutputVal();
        m_error += delta * delta;
        
    }
    
    m_error /= outputLayer.size() - 1; // get average error squared
    m_error = sqrt(m_error); //  RMS
    
    // implement a recent average average measurement (optional)
    
    m_recentAverageError = (m_recentAverageError * m_recentAverageSmoothingFactor + m_error) / (m_recentAverageSmoothingFactor + 1.0);
    
    // calculate output layer gradients
    
    for (unsigned n = 0; n < outputLayer.size() - 1; n++){
        
        outputLayer[n].calcOutputGradients(targetVals[n]);
        
    }
    
    // calculate gradients on hidden layers
    
    for (unsigned layerNum = m_layers.size() - 2; layerNum > 0; layerNum--){
        
        Layer &hiddenLayer = m_layers[layerNum];
        Layer &nextLayer = m_layers[layerNum + 1];
        
        for (unsigned n = 0; n < hiddenLayer.size(); n++){
            
            hiddenLayer[n].calcHiddenGradients(nextLayer);
            
        }
        
    }
    
    // for al laters form outputs to first hidden lauer
    // update connection weights
    
    for  (unsigned layerNum = m_layers.size() - 1; layerNum > 0; layerNum--){
        
        Layer &layer = m_layers[layerNum];
        Layer &prevLayer = m_layers[layerNum - 1];
        
        for (unsigned n = 0; n < layer.size() - 1; n++){
            
            layer[n].updateInputWeights(prevLayer);
            
        }
        
    }
    
}

void Net::displayNet(){
    
    glPushMatrix();
    glTranslatef(100.0f, 10.0f, 0.0f);
    
    // loop though layers
    for  (unsigned layerNum = 0 ; layerNum < m_layers.size(); layerNum++){
        
        Layer &layer = m_layers[layerNum];
        
        glPushMatrix();
        
        gl::color(0.0f, 0.2f, 0.3f);
        
        // loop through neurons
        if ( layerNum != m_layers.size()){
            for (unsigned n = 0; n < layer.size(); n++){
                glTranslatef(0.0f, 50.0f, 0.0f);
                
                for (unsigned c = 0; c < layer[n].m_outputWeights.size(); c++){
                    
                    gl::color(0.0f, 0.2f, 0.25f);
                    gl::lineWidth( (float)layer[n].m_outputWeights[c].weight * 6);
//                    gl::lineWidth( 1.0f);
                    gl::drawLine( Vec2f( 0.0f, 0.0f ), Vec2f( 350.0f, 50.0f * c  - 50.0f * n) );
                    
                }
                
                // changes the color of the bias neurons
                if ( n == layer.size()-1 ){
                    gl::color(0.0f, 0.3f, 0.2f);
                } else {
                    gl::color(0.0f, 0.2f, 0.3f);
                }
                
                gl::drawSolidEllipse(cinder::Vec2f(0.0f, 0.0f), 10.0f, 10.0f);
                
            }
        }
        glPopMatrix();
        
        glTranslatef(350.0f, 0.0f, 0.0f);
    }
    glPopMatrix();
    
    gl::lineWidth( 1.0f);
    
}