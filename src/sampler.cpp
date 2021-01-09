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

#include "sampler.h"

static LSGraph *globGraph = nullptr;
static RWModel *randomWalkModel = nullptr;
myrandom SamplerManager::random = myrandom(time(0) + mainrandom.irand(10000));

Sampler::Sampler() {
    this->previousSample = 0;
    this->started = false;
}

MemSampler::MemSampler(): Sampler() {
    this->previousWeight = 0.0;
}

void Sampler::initialize(Sampler *sampler, State state, StartMode mode, myrandom &random, bool mem) {
    int vertex = state.first;
    int offset = state.second;
    EdgeIndexType curDegree = globGraph->getDegree()[vertex];
    EdgeIndexType curOffset = globGraph->getOffsets()[vertex];
    
    /* Random initialization */
    if (mode == RANDOM) {
        EdgeIndexType nextEdgeIdx = curOffset + (EdgeIndexType)(random.irand(curDegree));
        float nextWeight = randomWalkModel->computeWeight(
            state, nextEdgeIdx);
        int cnt = 0;
        int i;
        
        /* Handle dead vertex */
        if (nextWeight <= 0) {
            for (i = 0; i < curDegree; i++) {
                nextEdgeIdx = curOffset + i;
                nextWeight = randomWalkModel->computeWeight(
                    state, nextEdgeIdx);
                if (nextWeight > 0) break;
            }
            if (i == curDegree - 1) throw vertex;
        }

        sampler->previousSample = nextEdgeIdx;
        if (mem) {
            static_cast<MemSampler *>(sampler)->previousWeight = nextWeight;
        }
    } 
    /* Burn-in initialization */
    else if (mode == BURNIN) {
        Sampler::initialize(sampler, state, RANDOM, random, mem);
        float w, w1;
        EdgeIndexType nextEdgeIdx; 
        for (int iter = 0; iter < 100; iter++) {
            nextEdgeIdx = curOffset + (EdgeIndexType)random.irand(curDegree);
            Sampler::getSample(sampler, state, nextEdgeIdx, random, mem);
        }
    }
    /* High weight initialization */
    else if (mode == WEIGHT) {
        Sampler::initialize(sampler, state, RANDOM, random, mem);
        EdgeIndexType nextEdgeIdx; 
        
        EdgeIndexType maxEdge = sampler->previousSample;
        float maxWeight;
        if (mem) {
            maxWeight = static_cast<MemSampler *>(sampler)->previousWeight;
        } else {
            maxWeight   = randomWalkModel->computeWeight(state, maxEdge);
        }
        //float maxWeight   = randomWalkModel->computeWeight(state, maxEdge);
        float w1;
        for (int iter = 0; iter < 20; iter++) {
            nextEdgeIdx = curOffset + (EdgeIndexType)random.irand(curDegree);
            w1 = randomWalkModel->computeWeight(
                state, nextEdgeIdx);
            if (w1 > maxWeight) {
                maxWeight = w1;
                maxEdge = nextEdgeIdx;
            }
        }
        sampler->previousSample = maxEdge;
        if (mem) {
            static_cast<MemSampler *>(sampler)->previousWeight - maxWeight;
        }
    }
}

EdgeIndexType Sampler::getSample(Sampler *sampler, State curState, long long candidateSample, myrandom &random, bool mem) {
    float newWeight = randomWalkModel->computeWeight(curState, candidateSample);
    float prevWeight;
    if (!mem) {
        prevWeight = randomWalkModel->computeWeight(curState, sampler->previousSample);
    } else {
        prevWeight = static_cast<MemSampler *>(sampler)->previousWeight;
    }
    if (sampler->accept(prevWeight, newWeight, random)) {
        sampler->previousSample = candidateSample;
        if (mem) {
            static_cast<MemSampler *>(sampler)->previousWeight = newWeight;
        }
    }
    return sampler->previousSample;
}

/*
 * Accept the candidate result with probability w1 / w
 */
bool Sampler::accept(float w, float w1, myrandom &rand) {
    if (w < w1) return true;
    return rand.drand() <= (double)(w1) / (double)(w); 
}

SamplerManager::SamplerManager(
        RWModel *model, LSGraph *graph) {
    globGraph = graph;
    this->graph = graph;
    randomWalkModel = model;
    this->vertexNum = this->graph->getNumberOfVertex();
    this->memWeight = true;
    /* allocate bucket array */
    this->samplerSet = static_cast<Sampler **>(
        malloc(this->graph->getNumberOfVertex() * sizeof(Sampler *))
    );

    MemSampler *a = new MemSampler();
    cout << "SIZE " << sizeof(*a) << endl;

    /* allocate sampler buckets for each vertex */
    for (int vertex = 0; vertex < this->vertexNum; ++vertex) {
        int bucketSize = model->stateNum(vertex);

        if (this->memWeight) {
            this->samplerSet[vertex] = (Sampler *)(new MemSampler[bucketSize]);
        } else {
            this->samplerSet[vertex] = new Sampler[bucketSize];
        }
    }
}

EdgeIndexType SamplerManager::getNextEdge(State curState, long long candidateSample, StartMode startMode, myrandom &random, bool mem) {
    int vertex = curState.first;
    int offset = curState.second;
    Sampler *sampler = static_cast<Sampler *>(&this->samplerSet[vertex][offset]);
    if (!sampler->started) {
        Sampler::initialize(
            sampler, curState, startMode, random, this->memWeight);
    }
    return Sampler::getSample(sampler, curState, candidateSample, random, mem);
}