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

#include "models/metapath.h"

#include <assert.h>

Metapath2vec::Metapath2vec(LSGraph *_graph, int argc, char **argv) {
    this->graph = _graph;
    this->metaLength = 0;
    this->walkLength = 80;

    this->getArgs(argc, argv);

    this->init();
}

void Metapath2vec::init() {
    this->edges = graph->getEdges();
    this->node_types = graph->getTypes();
    this->offsets = graph->getOffsets();
    this->weights = graph->getWeights();
}

float Metapath2vec::computeWeight(State curState, long long nextEdgeIndex) {
    int prevPosition = curState.second;
    int curVertex = curState.first;
    //cout << curVertex << endl;
    if (node_types[edges[nextEdgeIndex]] == metapath[(prevPosition + 1) % 4]) {
        return weights[nextEdgeIndex];
    } else {
        
        return 0;
    }
}

State Metapath2vec::newState(State curState, long long nextEdgeIndex) {
    int nextPosition = (curState.second + 1) % 4;
    int nextVertex = edges[nextEdgeIndex];
    return std::make_pair(nextVertex, nextPosition);
}

State Metapath2vec::getInitialState(int initialVertex) {
    if (node_types[initialVertex] != metapath[0]) {

        return std::make_pair(-1, 0);
    }
    return std::make_pair(initialVertex, 0);
}

int Metapath2vec::stateNum(int vertex) {
    return 5;
}

void Metapath2vec::parseMeta() {
    bool bad = false;
    int len = this->metaString.length();

    /* Legal metapath check */
    for (auto c : this->metaString) {
        if (!(c > '0' && c <= '9')) {
            bad = false;
            break;
        }
    }
    assert(bad == false);

    /* Metapath should be loop */
    assert(this->metaString[0] == this->metaString[len - 1]);

    this->metapath = static_cast<int *>(malloc(this->metaLength * sizeof(int)));

    for (int idx = 0; idx < len; idx++) {
        this->metapath[idx] = this->metaString[idx] - '0';
        assert(this->metapath[idx] <= this->graph->getTypeNum());
    }

}

void Metapath2vec::getArgs(int argc, char **argv) {
    int a = 0;
    
    if ((a = argPos(const_cast<char *>("-meta"), argc, argv)) > 0) {
        this->metaString = std::string(argv[a + 1]);
        this->parseMeta();
    }
    /* Metapath must be provided */
    assert(a > 0);
    
    if ((a = argPos(const_cast<char *>("-length"), argc, argv)) > 0) {
        this->length = std::atoi(argv[a + 1]);
    }
        
}