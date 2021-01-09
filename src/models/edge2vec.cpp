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

#include "models/edge2vec.h"

Edge2vec::Edge2vec(LSGraph *_graph, int argc, char **argv) {
    this->graph = _graph;
    assert(graph->isHetro());
    this->type_num = graph->getTypeNum();
    this->param_p = 1.0f;
    this->param_q = 1.0f;
    this->iterNum = 4;
    this->walkLength = 80;
    this->init();
}
/*
 * map a pair of node types onto a edge type
 */
int Edge2vec::edgeType(int v1, int v2) {
    return (v1 - 1) * this->type_num + v2 - 1;
}

void Edge2vec::init() {
    this->edges = graph->getEdges();
    this->node_types = graph->getTypes();
    this->offsets = graph->getOffsets();
    this->weights = graph->getWeights();
    this->degrees = graph->getDegree();
    //this->edges_r = graph->getEdges_r();
    this->vertexNum = graph->getNumberOfVertex();

    this->edge_type_num = this->edgeType(type_num, type_num) + 1;

    this->matM = static_cast<float **>(
        malloc(this->edge_type_num * sizeof(float *)));
    for (int i = 0; i < this->edge_type_num; i++) {
        this->matM[i] = static_cast<float *>(
            malloc(this->edge_type_num * sizeof(float)));
        for (int j = 0; j < this->edge_type_num; j++)
            this->matM[i][j] = 1.0;
    }

    init_sigmoid_table();

    this->typeCount = static_cast<int **>(
        malloc(this->edge_type_num * sizeof(int *)));
    for (int i = 0; i < this->edge_type_num; i++) {
        this->typeCount[i] = static_cast<int *>(
            malloc(vertexNum * sizeof(int)));
    }
}

std::pair<int, int> Edge2vec::nodeType(int edge) {
    return std::make_pair(edge / this->type_num + 1, edge % this->type_num + 1);
}

/*
 * update edge type counting
 **/
void Edge2vec::handleWalk(int *walkSeq, int length) {
    //cout << "WALK" << endl;
    //cout << walkSeq[0] << endl;
    int *count = static_cast<int *>(malloc(this->edge_type_num * sizeof(int)));
    memset(count, 0, sizeof(int) * edge_type_num);
    for (int i = 0; i < walkLength - 1; i++) {
        int curType = this->edgeType(node_types[walkSeq[i]], node_types[walkSeq[i + 1]]);
        count[curType]++;
    }
    for (int i = 0; i < this->edge_type_num; i++) {
        this->typeCount[i][walkSeq[0]] = count[i];
    }
    free(count);
}
/*
 * update matrix M after an iteration
 **/
void Edge2vec::handleIter() {
    long long *flatSum = static_cast<long long *>(malloc(this->edge_type_num * sizeof(long long)));
    long long *squareSum = static_cast<long long *>(malloc(this->edge_type_num * sizeof(long long)));

    for (int i = 0; i < this->edge_type_num; i++) {
        flatSum[i] = 0;
        squareSum[i] = 0;
        for (int j = 0; j < vertexNum; j++) {
            flatSum[i] += (long long)typeCount[i][j];
            //cout << flatSum[i] << endl;
            squareSum[i] += (long long)(typeCount[i][j] * typeCount[i][j]);
        }
    }
    /* pearson correlation */
    for (int i = 0; i < this->edge_type_num; i++) {
        for (int j = 0; j <= i; j++) {
            if (flatSum[i] == 0 || flatSum[j] == 0) continue;
            long long prodSum = 0;
            for (int walkIdx = 0; walkIdx < vertexNum; walkIdx++)
                prodSum += (long long)(typeCount[i][walkIdx] * (long long)typeCount[j][walkIdx]);
            long long nume = (long long)vertexNum * (long long)prodSum - (long long)flatSum[i] * (long long)flatSum[j];
            double denom = sqrt(double(
                (vertexNum * squareSum[i] - flatSum[i] * flatSum[i]) *
                (vertexNum * squareSum[j] - flatSum[j] * flatSum[j])
            ));
            if (isnan(denom)) continue;
            if (denom != 0.0)
                this->matM[i][j] = fast_sigmoid(double(nume) / denom);
        }
    }

    for (int i = 0; i < this->edge_type_num; i++) {
        for (int j = i + i; j < this->edge_type_num; j++) {
            this->matM[i][j] = this->matM[j][i];
        }
    }
}

float Edge2vec::computeWeight(State curState, long long nextEdgeIndex) {
    int curVertex = curState.first;
    int prevVertex = edges[offsets[curVertex] + curState.second];
    int nextVertex = edges[nextEdgeIndex];
    float factorM = this->matM[edgeType(node_types[prevVertex], node_types[curVertex])]
                              [edgeType(node_types[curVertex], node_types[nextVertex])];
    float alpha = 1.0f;
    if (param_q != 1.0f || param_q != 1.0f) {
        if (prevVertex == nextVertex) 
            alpha = 1.0f / param_p;
        else if (graph->has_edge(prevVertex, nextVertex)) 
            alpha = 1.0f / param_q;
        else alpha = 1.0f;
    }
    return factorM * alpha * weights[nextEdgeIndex];
}

State Edge2vec::newState(State curState, long long nextEdgeIndex) {
    VertexIndexType nextV = edges[nextEdgeIndex];
    VertexIndexType curV = curState.first;
    int revOffset = graph->getRevEdge(curV, nextEdgeIndex) - offsets[nextV];
    return std::make_pair(nextV, revOffset);
}

State Edge2vec::getInitialState(int initialVertex) {
    int degree = degrees[initialVertex];
    long long randOffset = (long long)random.irand(degree);
    return std::make_pair(initialVertex, randOffset);
}


int Edge2vec::getIter() {
    return this->iterNum;
}

void Edge2vec::getArgs(int argc, char **argv) {
    int a = 0;
    if ((a = argPos(const_cast<char *>("-p"), argc, argv)) > 0)
        this->param_p = atof(argv[a + 1]);
    if ((a = argPos(const_cast<char *>("-q"), argc, argv)) > 0)
        this->param_q = atof(argv[a + 1]);
}

int Edge2vec::stateNum(int vertex) {
    return this->degrees[vertex];
}

