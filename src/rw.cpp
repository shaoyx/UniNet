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

#include "rw.h"

#include <iostream>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>

RandomWalk::RandomWalk(LSGraph *_graph, int _argc, char **_argv) {
    this->graph = _graph;
    this->threadNum = 16;
    this->nodeWNum = 10;
    getArgs(_argc, _argv);
    cout << "Walks per node: " << nodeWNum << endl;
    this->walkNum = graph->getNumberOfVertex() * nodeWNum;
    this->argc = _argc;
    this->argv = _argv;
    RWModel *model = this->init();

    /* sampler management */
    this->samplerManager = new SamplerManager(model, this->graph);

    this->runModel(model);
}

/**
 * Initialize a random walk model and returns `RWModel` typed pointer.
 */
RWModel *RandomWalk::init() {
    RWModel *model = nullptr;

    if (this->type == DEEPWALK) {
        DeepWalk *deepWalk = new DeepWalk(graph);
        model = (RWModel *)deepWalk;
        std::cout << "DeepWalk" << std::endl;
    } else if (this->type == NODE2VEC) {
        Node2vec *node2vec = new Node2vec(graph, argc, argv);
        model = (RWModel *)node2vec;
        std::cout << "Node2vec" << std::endl;
    } else if (this->type == METAPATH) {
        Metapath2vec *metapath2vec = new Metapath2vec(graph, argc, argv);
        model = (RWModel *)metapath2vec;
        std::cout << "Metapath2vec" << std::endl;
    } else if (this->type == FAIRWALK) {
        Fairwalk *fairwalk = new Fairwalk(graph, argc, argv);
        model = (RWModel *)fairwalk;
        std::cout << "Fairwalk" << std::endl;
    } else if (this->type == EDGE2VEC) {
        Edge2vec *edge2vec = new Edge2vec(graph, argc, argv);
        model = (RWModel *)edge2vec;
        std::cout << "Edge2vec" << std::endl;
    }
    return model;
} 

void RandomWalk::runModel(RWModel *model) {
    int walkLength = model->getWalkLength();
    auto begin = chrono::steady_clock::now();
    int vertexNum = graph->getNumberOfVertex();
    char outputPath[100];
    char outputtxt[100];

    FILE *files[40];
    FILE *txt[40];
    memset(files, 0, sizeof(files));
    int iterNum = model->getIter();
    int totalNum = iterNum * walkNum;

    int cnt = 0;
    if (this->out) {
        std::string prefix = "txt";
	    if (access(prefix.c_str(), 0) == -1)
		    mkdir(prefix.c_str(), S_IRWXU);
        else 
            int stat = system("rm txt/*");
        for (int i = 0; i < threadNum; i++) {
            sprintf(outputPath, "./walks/rw_out_%d", i);
            files[i] = fopen(outputPath, "wb");
            sprintf(outputtxt, "./txt/rw_out_%d", i);
            txt[i] = fopen(outputtxt, "w");
        }
    }

    /* Edge2vec requires multiple iterations */
    int iteration = model->getIter();
    myrandom rand = myrandom(time(0) + mainrandom.irand(10000));
    for (int iter = 1; iter <= iteration; iter++) {

#pragma omp parallel for num_threads(threadNum)
        for (long long i = 1; i < walkNum; i++) {
            int tid = omp_get_thread_num();

            VertexIndexType startVertex = i % vertexNum;
            int startState = rand.irand(model->stateNum(startVertex));

            State initialState = std::make_pair(startVertex, startState);

            /* Create walker instance */
            Walker walker(
                model,                  /* random walk model */
                graph,                  /* graph pointer */
                walkLength,             /* random walk length */
                (int)(i % vertexNum),   /* starting vertex */
                initialState,
                this->startMode,        /* initialization strategy */
                txt[tid],             /* output file pointer */
                this->out,              /* do we output walk sequence */
                this->samplerManager
            );

            /* Count the total walker number */
            cnt++;

            /* Display walking workload progress every now and then */
            if (tid == 0 && cnt % 100 == 0) {
                std::cout << fixed << setprecision(2) << "\rProgress "  << cnt * 100.f / (totalNum + 1) << "%";
            }
            walker.walkerExecute();
        }

        if (this->out && iter == iteration) {
            for (int i = 0; i < threadNum; i++) {
                fclose(txt[i]);
            }
        }

    }

    auto end = chrono::steady_clock::now();
    
    std::cout << "\rWalks generation took "
        << chrono::duration_cast<chrono::duration<float>>(end - begin).count()
        << " s to run" << endl;

    if (this->out) {
        cout << "merge" << endl;
        int stat = system("cat txt/rw_* >> txt/all");
        if (stat) {
            cerr << stat << endl;
        }
        stat = system("rm txt/rw_*");
        if (stat) {
            cerr << stat << endl;
        }
        cout << "finish merge" << endl;
    }
    
}

void RandomWalk::getArgs(int argc, char **argv) {
    int a = 0;
    if ((a = argPos(const_cast<char *>("-deepwalk"), argc, argv)) > 0)
        this->type = DEEPWALK;
    else if ((a = argPos(const_cast<char *>("-node2vec"), argc, argv)) > 0)
        this->type = NODE2VEC;
    else if ((a = argPos(const_cast<char *>("-metapath"), argc, argv)) > 0)
        this->type = METAPATH;
    else if ((a = argPos(const_cast<char *>("-fairwalk"), argc, argv)) > 0)
        this->type = FAIRWALK;
    else if ((a = argPos(const_cast<char *>("-edge2vec"), argc, argv)) > 0)
        this->type = EDGE2VEC;

    if ((a = argPos(const_cast<char *>("-out"), argc, argv)) > 0)
        this->out = true;
    else this->out = false;

    if ((a = argPos(const_cast<char *>("-train"), argc, argv)) > 0)
        this->out = true;
    if ((a = argPos(const_cast<char *>("-threads"), argc, argv)) > 0)
        this->threadNum = atoi(argv[a + 1]);
    if ((a = argPos(const_cast<char *>("-walks"), argc, argv)) > 0)
        this->nodeWNum = atoi(argv[a + 1]);

    if ((a = argPos(const_cast<char *>("-burnin"), argc, argv)) > 0)
        this->startMode = BURNIN;
    else if ((a = argPos(const_cast<char *>("-weight"), argc, argv)) > 0)
        this->startMode = WEIGHT;
    else this->startMode = RANDOM;
    
}