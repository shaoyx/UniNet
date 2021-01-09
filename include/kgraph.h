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

#ifndef KGRAPH_H
#define KGRAPH_H

#include <string.h>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "utils.h"

using namespace std;

typedef long long   EdgeIndexType;
typedef int         VertexIndexType;
typedef float       WeightType;

/** 
 * large scale graph
 * The graph storage model should be updated, in order to store large scale labeled weighted graphs.
 * Here we assume node has types, edge has types and weights.
 **/

class LSGraph {
private:
    EdgeIndexType   *offsets;
    VertexIndexType *edges;
    VertexIndexType *degrees;
    VertexIndexType *node_types;
    float *weights;

    bool            tossWeight;
    bool            tossReverse;
    
    bool            hetro;
    VertexIndexType nv;
    EdgeIndexType   ne;
    VertexIndexType type_num;
    EdgeIndexType   *edges_r;
    
public:
    bool weighted; 
    LSGraph(){};
    ~LSGraph() {
        free(this->offsets);
        free(this->edges);
        free(this->degrees);
        free(this->weights);
        free(this->edges_r);
    }
    bool loadCRSGraph(string network_file);
    bool loadCRSGraph(int argc, char **argv);

    void printGraphInfo();
    
    void init_reverse();

    EdgeIndexType find_edge(int src, int dst);

    int has_edge(int from, int to);

    long long getNumberOfVertex();

    EdgeIndexType getNumberOfEdge();

    bool isHetro();

    int* getDegree();

    EdgeIndexType *getOffsets();

    int* getEdges();

    int *getTypes();

    int getTypeNum();

    float* getWeights();

    EdgeIndexType *getEdges_r();

    EdgeIndexType getRevEdge(VertexIndexType src, EdgeIndexType idx);

    WeightType getEdgeWeight(EdgeIndexType idx);
};

#endif