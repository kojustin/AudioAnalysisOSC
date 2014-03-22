//
//  Net.h
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 14/03/2014.
//
//

#include <iostream>
#include <vector>

using namespace std;

#include "Neuron.h"

typedef vector<Neuron> Layer;

class Net{
    
public:
    Net();
    Net( const vector<unsigned> &topology);
    void feedForward( const vector<float> &inputVals);
    void backProp( const vector<float> &targetVals);
    void getResults(vector<float> &resultVals) const;
    
    void displayNet();
    
private:
    vector<Layer> m_layers; // m_layers[layerNum][neuroNum]
    float m_error;
    float m_recentAverageError;
    float m_recentAverageSmoothingFactor;
};
