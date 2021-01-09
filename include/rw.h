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

#ifndef RW_H
#define RW_H

#include "models/deepwalk.h"
#include "models/node2vec.h"
#include "models/metapath.h"
#include "models/fairwalk.h"
#include "models/edge2vec.h"
#include "walker.h"

#include <omp.h>
#include <chrono>
#include <iomanip>

enum ModelType {
    DEEPWALK,
    NODE2VEC,
    METAPATH,
    FAIRWALK,
    EDGE2VEC
};

class RandomWalk {
public:
    RandomWalk(LSGraph *_graph, int argc, char **argv);
private:
    LSGraph *graph;
    ModelType type;
    int nodeWNum;
    long long walkNum;
    int argc;
    char **argv;
    bool out;
    int threadNum;
    StartMode startMode;

    SamplerManager *samplerManager;

    RWModel *init();

    void runModel(RWModel *model);

    void getArgs(int argc, char **argv);
};

#endif