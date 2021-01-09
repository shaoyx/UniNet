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

#include "models/node2vec.h"

#include <iostream>
#include <algorithm>

Node2vec::Node2vec(LSGraph *_graph, int argc, char **argv) {
    graph = _graph;
    init();
    getArgs(argc, argv);
    this->max_weight = std::max(1.0f, 1.0f / this->paramQ);
    this->max_weight = std::max(1.0f, 1.0f / this->paramP);
    std::cout << "init node2vec" << std::endl;
}

void Node2vec::init() {
    this->edges = graph->getEdges();
    this->edges_r = graph->getEdges_r();
    this->weights = graph->getWeights();
    this->degrees = graph->getDegree();
    this->offsets = graph->getOffsets();
    this->walkLength = 80;
    paramP = 0.25;
    paramQ = 0.25;
    
}

float Node2vec::computeWeight(State curState, long long nextEdgeIndex) {
    long long curEdge = this->offsets[curState.first] + curState.second;
    int src = edges[curEdge];
    int nextV = edges[nextEdgeIndex];
    float nextW = weights[nextEdgeIndex];
    if (src == nextV) {
        return nextW / paramP;
    } else if (graph->has_edge(src, nextV)) {
        return nextW;
    } else {
        return nextW / paramQ;
    }
}

State Node2vec::newState(State curState, long long nextEdgeIndex) {
    int nextV = edges[nextEdgeIndex];
    int revOffset = edges_r[nextEdgeIndex] - offsets[nextV];
    return std::make_pair(nextV, revOffset);
}

State Node2vec::getInitialState(int initialVertex) {
    long long degree = degrees[initialVertex];
    long long randOffset = (long long)random.irand(degree);
    return std::make_pair(initialVertex, randOffset);
}

void Node2vec::getArgs(int argc, char **argv) {
    int a = 0;
    if ((a = argPos(const_cast<char *>("-p"), argc, argv)) > 0)
        this->paramP = atof(argv[a + 1]);
    if ((a = argPos(const_cast<char *>("-q"), argc, argv)) > 0)
        this->paramQ = atof(argv[a + 1]);
}

int Node2vec::stateNum(int vertex) {
    return this->degrees[vertex];
}

float Node2vec::maxWeight() {
    return this->max_weight;
}
