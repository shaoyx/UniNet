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

#include "rwmodel.h"

#include <string>

class Metapath2vec : RWModel {
public:
    Metapath2vec(LSGraph *_graph, int argc, char **argv);

    float computeWeight(State curState, long long nextEdgeIndex);
    State newState(State curState, long long nextEdgeIndex);
    State getInitialState(int initialVertex);
    int stateNum(int vertex);

    int getLength() { return this->length; }
    int *getMetapath() { return this->metapath; }
private:
    void init();

    int length;
    int *metapath;

    int *edges;
    int *node_types;
    long long *offsets;
    float *weights;

    std::string metaString;
    int metaLength;

    void parseMeta();

    void getArgs(int argc, char **argv);
};