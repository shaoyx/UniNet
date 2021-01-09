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

#include "walker.h"
#include <iostream>

Walker::Walker(
        RWModel *_model, 
        LSGraph *_graph,
        int     _walkLength,
        int     _initialVertex,
        State   _initialState,
        StartMode _startMode,
        FILE    *file,
        bool    _out,
        SamplerManager  *_samplerManager,
        int     _burninIter) {
    this->randomWalkModel   = _model;
    this->graph             = _graph;
    this->walkLength        = _walkLength;
    this->initialVertex     = _initialVertex;
    this->startMode         = _startMode;
    this->burninIter        = _burninIter;
    this->executable        = true;
    //this->initialState      = this->randomWalkModel->getInitialState(
    //    this->initialVertex);
    this->initialState      = _initialState;
    this->fp                = file;
    this->out               = _out;
    this->samplerManager    = _samplerManager;
    if (initialState.first == -1) 
        this->executable = false;
    this->init();
} 

void Walker::init() {
    this->walkSq = static_cast<int *>(malloc(this->walkLength * sizeof(int)));
    this->setGraph();
}

void Walker::setGraph() {
    this->edges = graph->getEdges();
    this->offsets = graph->getOffsets();
    this->degrees = graph->getDegree();
}

Walker::~Walker() {
    free(walkSq);
}

void Walker::walkerExecute() {
    if (!this->executable) return;

    this->walkSq[0] = this->initialVertex;
    this->curState = this->initialState;
    this->curVertex = this->initialVertex;

    float w, w1;
    long long nextEdgeIdx;
    long long prevEdgeIdx;
    long long curDegree;
    long long curOffset;

    /* Main loop for walker execution */
    for (int i = 1; i < this->walkLength; i++) {
        int vertex = this->curState.first;
        int index = this->curState.second;

        curDegree = degrees[this->curVertex];
        curOffset = offsets[this->curVertex];

        nextEdgeIdx = curOffset + (long long)this->random.irand(curDegree);
        nextEdgeIdx = this->samplerManager->getNextEdge(
            this->curState, nextEdgeIdx, this->startMode, random, this->samplerManager->memWeight);

        this->curVertex = edges[nextEdgeIdx];
        this->walkSq[i] = this->curVertex;

        this->curState = this->randomWalkModel->newState(
            this->curState, nextEdgeIdx);

    }

    this->randomWalkModel->handleWalk(this->walkSq, this->walkLength);

    if (this->out) {
        fwrite(this->walkSq, sizeof(int), this->walkLength, this->fp);
        for (int i = 0; i < this->walkLength; i++) {
            fprintf(this->fp, "%d ", this->walkSq[i]);
        }
        fprintf(this->fp, "\n");
    }
}

