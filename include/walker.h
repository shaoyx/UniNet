/**
 * MIT License
 * 
 * Copyright (c) 2020, Beijing University of Posts and Telecommunications.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#ifndef WALKER_H
#define WALKER_H

#include "rwmodel.h"
#include "utils.h"
#include "sampler.h"

#include <omp.h>
#include <unordered_map>

class Walker {
public:
    Walker(
        RWModel *_model, 
        LSGraph *_graph,
        int _walkLength,
        int _initialVertex,
        State _initialState,
        StartMode _startMode,
        FILE *file,
        bool _out,
        SamplerManager *_samplerManager,
        int _burninIter = 100
    );
     ~Walker();
    void walkerExecute();
private:

    LSGraph *graph;
    RWModel *randomWalkModel;

    SamplerManager *samplerManager;

    State initialState;
    State curState;
    bool weighted;
    float maxWeight;
    int initialVertex;
    int curVertex;
    int walkLength;
    char outputPath[100];
    FILE *fp;
    int *walkSq;
    FILE *txt;
    myrandom random = myrandom(time(0) + mainrandom.irand(10000));
    StartMode startMode;
    int burninIter;
    int tid;

    int *edges;
    long long *offsets;
    int *degrees;

    bool executable;
    bool out;

    void init();
    void setGraph();
    static bool accept(float w, float w1, myrandom &rand);
    //void samplerInit(State state, int vertex, StartMode mode);
};


#endif // WALKER_H