#include "utils.h"
#include "kgraph.h"
#include <chrono>
#include <omp.h>
#include <iomanip>

#define VECTORIZE 1

using ull = unsigned long long;

class Train {
public:
    Train(LSGraph *_graph, int argc, char **argv);

private:
    FILE *files[30];

    void trainSG();
    void init();
    void write_file();

    inline void update( // update the embedding, putting w_t gradient in w_t_cache
        float *w_s, float *w_t, float *w_t_cache, float lr, const int label);

    float *wVtx;
    float *wCtx;
    float initial_lr;
    int window_size;
    int n_hidden;
    int n_walks;
    int walk_length;
    char *out_path;

    int threadNum;

    int nv;
    ull step;

    void getArgs(int argc, char **argv);
    
};