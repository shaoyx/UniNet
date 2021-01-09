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

#include "models/fairwalk.h"

Fairwalk::Fairwalk(LSGraph *_graph, int argc, char **argv) {
    this->graph = _graph;
    this->type_num = graph->getTypeNum();

    this->param_p = 1.0;
    this->param_q = 1.0;

    this->init();
}

void Fairwalk::init() {

    this->edges = graph->getEdges();
    this->node_types = graph->getTypes();
    this->offsets = graph->getOffsets();
    this->weights = graph->getWeights();
    this->vertexNum = graph->getNumberOfVertex();
    this->degrees = graph->getDegree();

    this->preProc();
}

// get total number of each node type
void Fairwalk::preProc() {
    this->neighborAttr = static_cast<int **>(
        malloc(graph->getNumberOfVertex() * sizeof(int *)));
    for (int i = 0; i < this->vertexNum; i++) {
        this->neighborAttr[i] = static_cast<int *>(
            malloc((this->type_num + 1) * sizeof(int32_t)));
        for (int type = 1; type <= this->type_num; type++) {
            int cnt = 0;
            for (long long offset = 0; offset < degrees[i]; offset++) {
                int neighborIndex = edges[offsets[i] + offset];
                if (node_types[neighborIndex] == type) cnt++;
            }
            this->neighborAttr[i][type] = cnt;
        }
    }
}

float Fairwalk::computeWeight(State curState, long long nextEdgeIndex) {
    cout << "F" << endl;
    int curVertex = curState.first;
    int prevVertex = edges[offsets[curVertex] + curState.second];
    int nextVertex = edges[nextEdgeIndex];

    int nextType = node_types[edges[nextEdgeIndex]];

    float alpha = 1.0f;
    if (param_q != 1.0f || param_q != 1.0f) {
        if (prevVertex == nextVertex) 
            alpha = 1.0f / param_p;
        else if (graph->has_edge(prevVertex, nextVertex)) 
            alpha = 1.0f / param_q;
        else alpha = 1.0f;
    }

    return (float)weights[nextEdgeIndex] * alpha / 
           (float)neighborAttr[curVertex][nextType]; 
}

State Fairwalk::newState(State curState, long long nextEdgeIndex) {
    VertexIndexType nextV = edges[nextEdgeIndex];
    VertexIndexType curV = curState.first;
    int revOffset = graph->getRevEdge(curV, nextEdgeIndex) - offsets[nextV];
    return std::make_pair(nextV, revOffset);
}

State Fairwalk::getInitialState(int initialVertex) {
    return std::make_pair(initialVertex, 0);
}

int Fairwalk::stateNum(int vertex) {
    return 1;
}

void Fairwalk::getArgs(int argc, char **argv) {
    int a = 0;
    if ((a = argPos(const_cast<char *>("-arg"), argc, argv)) > 0) ;
        
}

