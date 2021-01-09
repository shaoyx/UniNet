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

#include "kgraph.h"
#include "rwmodel.h"

/* Edge sampler initialization strategy */
enum StartMode {
    RANDOM,
    BURNIN,
    WEIGHT
};

/*
 * M-H based edge sampler.
 * Each sampler stores the previous sampled egde and its dynamic weight.
 **/
class Sampler {
public:
    Sampler();
    static EdgeIndexType getSample(Sampler *sampler, State curState, long long candidateSample, myrandom &random, bool mem);
    static void initialize(Sampler *sampler, State state, StartMode mode, myrandom &random, bool mem);
    /* Last sampled edge idx */
    EdgeIndexType   previousSample;
    bool        started;
    /* Last sampled dynamic weight */

    bool accept(float w, float w1, myrandom &rand);
};

class MemSampler : public Sampler {
public:
    MemSampler();
    WeightType     previousWeight;
};

/*
 * Sampler manager. 
 * We use 2-D layout to organize samplers
 */
class SamplerManager {
public:
    SamplerManager(RWModel *model, LSGraph *graph);

    /* find corresponding sampler based on the current state */
    EdgeIndexType getNextEdge(State curState, long long candidateSample, StartMode startMode, myrandom &random, bool mem);

    LSGraph *graph;
    static myrandom random;
    bool memWeight;
private:
    int vertexNum;

    /* Sampler matrix */
    Sampler **samplerSet;
    
};