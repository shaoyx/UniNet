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

#ifndef RWMODEL_H
#define RWMODEL_H

#include "kgraph.h"

using State = std::pair<VertexIndexType, int>;

class RWModel {
public:
    virtual float computeWeight(
        State curState, 
        EdgeIndexType nextEdgeIndex) = 0;
    
    virtual State newState(
        State curState,
        EdgeIndexType nextEdgeIndex) = 0;

    virtual State getInitialState(
        int initialVertex) = 0;
    
    virtual int stateNum(int vertex) = 0;

    LSGraph *getGraph() { return this->graph; }

    int getWalkLength() { return this->walkLength; }

    virtual float maxWeight() { return 99999.9; }

    virtual void handleaIter() {}

    virtual void handleWalk(int *walkSeq, int length) {}

    virtual int getIter() { return this->iteration; }

protected:
    int walkLength;
    LSGraph *graph;

    int iteration = 1;
};

#endif // RWMODEL_H