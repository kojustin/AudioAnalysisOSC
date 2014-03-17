//
//  Net.h
//  AudioAnalysisOSC
//
//  Created by Felix Faire on 14/03/2014.
//
//

#ifndef __AudioAnalysisOSC__Net__
#define __AudioAnalysisOSC__Net__

#include <iostream>
#include <vector>

using namespace std;

#endif /* defined(__AudioAnalysisOSC__Net__) */

#include "Neuron.h"

typedef vector<Neuron> Layer;

class Net{
    
public:
    Net( const vector<unsigned> &topology);
    void feedForward( const vector<double> &inputVals);
    void backProp( const vector<double> &targetVals);
    void getResults(vector<double> &resultVals) const;
    
private:
    vector<Layer> m_layers; // m_layers[layerNum][neuroNum]
    double m_error;
    double m_recentAverageError;
    double m_recentAverageSmoothingFactor;
};
