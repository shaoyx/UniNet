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

#include "models/deepwalk.h"

DeepWalk::DeepWalk(LSGraph *_graph) {
    this->walkLength = 80;
    graph = _graph;
    cout << "init deepwalk" << endl;
}

float DeepWalk::computeWeight(State curState, long long nextEdgeIndex) {
    //return this->graph->getWeights()[nextEdgeIndex];
    return 1.0;
}

State DeepWalk::newState(State curState, long long nextEdgeIndex) {
    int nextVertex = this->graph->getEdges()[nextEdgeIndex];
    return std::make_pair(nextVertex, 0);
}

State DeepWalk::getInitialState(int initialVertex) {
    return std::make_pair(initialVertex, 0);
}

int DeepWalk::stateNum(int vertex) {
    return 1;
}

float DeepWalk::maxWeight() {
    return 1.0;
}



