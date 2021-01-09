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

#include "train.h"

Train::Train(LSGraph *graph, int argc, char **argv) {
    this->nv = graph->getNumberOfVertex();
    threadNum = 1;
    
    init();
    this->getArgs(argc, argv);
    char inputPath[100];

    for (int i = 0; i < threadNum; i++) {
        sprintf(inputPath, "./walks/rw_out_%d", i);
        files[i] = fopen(inputPath, "rb");
    } 
    auto begin = chrono::steady_clock::now();
    this->trainSG();
    
    auto end = chrono::steady_clock::now();
    cout 
         << "\rEmbedding Training took "
         << chrono::duration_cast<chrono::duration<float>>(end - begin).count()
         << " s to run" << endl;
    if (this->out_path != nullptr) {
        cout << "Write embedding file" << endl;
        this->write_file();
    }
}

inline void *
aligned_malloc(size_t size,
               size_t align) { // universal aligned allocator for win & linux
#ifndef _MSC_VER
    void *result;
    if (posix_memalign(&result, align, size))
        result = 0;
#else
    void *result = _aligned_malloc(size, align);
#endif
    return result;
}

void Train::init() {
    initial_lr = 0.025f;
    window_size = 10;
    n_hidden = 128;
    n_walks = 10;
    step = 0;
    walk_length = 80;
    this->out_path = (char *)malloc(100 * sizeof(char));

    // each thread handles one walking file
    
    // vertex embedding

    this->wVtx = static_cast<float *>(
        aligned_malloc((nv + 1) * n_hidden * sizeof(float), DEFAULT_ALIGN));
    myrandom random(time(nullptr));  
    for (long long i = 0; i < (nv + 1) * n_hidden; i++)
        wVtx[i] = (random.drand() - 0.5) / n_hidden;
    // context embedding
    this->wCtx = static_cast<float *>(
        aligned_malloc((nv + 1) * n_hidden * sizeof(float), DEFAULT_ALIGN));
    memset(wCtx, 0, (nv + 1) * n_hidden * sizeof(float));

    init_sigmoid_table();
}

inline void Train::update( // update the embedding, putting w_t gradient in w_t_cache
    float *w_s, float *w_t, float *w_t_cache, float lr, const int label) {
    float score = 0; // score = dot(w_s, w_t)
    AVX_LOOP
    for (int c = 0; c < n_hidden; c++)
        score += w_s[c] * w_t[c];
    score = (label - fast_sigmoid(score)) * lr;
    AVX_LOOP
    for (int c = 0; c < n_hidden; c++)
        w_t_cache[c] += score * w_s[c]; // w_t gradient
    AVX_LOOP
    for (int c = 0; c < n_hidden; c++)
        w_s[c] += score * w_t[c]; // w_s gradient
}

void Train::trainSG() {
    ull total_steps = n_walks * nv;

#pragma omp parallel num_threads(threadNum)
{ 

    int tid = omp_get_thread_num();

if (files[tid] != nullptr)
{

    myrandom random(time(nullptr)); 
    const int trnd = random.irand(nv);
    ull ncount = 0;
    ull local_step = 0;
    float lr = initial_lr;
    int *dw_rw = static_cast<int *>(
        aligned_malloc(walk_length * sizeof(int),
                       DEFAULT_ALIGN)); // we cache one random walk per thread
    float *cache = static_cast<float *>(aligned_malloc(
        n_hidden * sizeof(float),
        DEFAULT_ALIGN)); // cache for updating the gradient of a node

#pragma omp barrier
    int now_cnt = 0;

    while (fread(dw_rw, sizeof(int), walk_length, files[tid]) > 0) {

        if (ncount > 10) { // update progress every now and then
#pragma omp atomic
            step += ncount;

            if (step > total_steps) // note than we may train for a little longer
                break;
            if (tid == 0 && step % 2 == 0)
                cout << fixed << setprecision(6) << "\rlr " << lr << ", Progress "
                     << setprecision(2) << step * 100.f / (total_steps + 1) << "%";
            ncount = 0;
            local_step = step;
            lr =
                initial_lr *
                (1 - step / static_cast<float>(total_steps + 1)); // linear LR decay
            if (lr < initial_lr * 0.0001)
                lr = initial_lr * 0.0001;
        }
        for (int dwi = 0; dwi < walk_length; dwi++) {
            int b = random.irand(window_size); // subsample window size
            long long n1 = dw_rw[dwi];
            if (n1 < 0)
                break;

            for (int dwj = max(0, dwi - window_size + b);
                dwj < min(dwi + window_size - b + 1, walk_length); dwj++) {
                if (dwi == dwj)
                    continue;
                long long n2 = dw_rw[dwi];
                if (n2 < 0)
                    break;
                if (n1 > nv || n2 > nv) continue;
                memset(cache, 0, n_hidden * sizeof(float)); // clear cache
                update(&wCtx[n1 * n_hidden], &wVtx[n2 * n_hidden], cache, lr, 1);

                AVX_LOOP
                for (int c = 0; c < n_hidden; c++)
                    wVtx[n2 * n_hidden + c] += cache[c];
            }
        }
        ncount++; 
    }
    free(dw_rw);
    free(cache);

} // if file exist

} // omp parallel threads
    
}

void Train::write_file() {
    FILE *out_file = fopen(this->out_path, "w");
    for (int i = 0; i < nv; i++) {
        fprintf(out_file, "%d", i);
        for (int j = 0; j < n_hidden; j++) {
            fprintf(out_file, " %f", wCtx[i * n_hidden + j]);
        }
        fprintf(out_file, "\n");
    }
    fclose(out_file);
}

void Train::getArgs(int argc, char **argv) {
    int a = 0;
    if ((a = argPos(const_cast<char *>("-threads"), argc, argv)) > 0)
        this->threadNum = atoi(argv[a + 1]);
    if ((a = argPos(const_cast<char *>("-walks"), argc, argv)) > 0)
        this->n_walks = atoi(argv[a + 1]);
    if ((a = argPos(const_cast<char *>("-embout"), argc, argv)) > 0)
        strcpy(this->out_path, argv[a + 1]);
}

