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

#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>

int *nodeid;
int *degrees;
int *nodeindex;
int **neighbors;

bool weighted = false;
bool rand_weight = false;
bool hetro = false;
bool rand_hetro = false;

std::string hetro_string;



int argPos(char *str, int argc, char **argv) {
    for (int i = 0; i < argc; ++i) {
        if (!strcmp(str, argv[i])) {
            if (i == argc - 1) {
                printf("Argument missing for %s\n", str);
                exit(1);
            }
            return i;
        }

    }
    return -1; 
}

void work(FILE *input, FILE *ot) {
    long long n, e;
    std::vector<int> xs, ys;

    int maxi = 0;
    int x, y;
    e = 0;

    while (fscanf(input, "%d%d", &x, &y) > 0) {
        e++;
        if (x > maxi) maxi = x;
        if (y > maxi) maxi = y;
        xs.push_back(x);
        ys.push_back(y);
    }

    std::cout << maxi << std::endl;
    n = maxi + 1;

    fclose(input);

    degrees = static_cast<int *>(malloc(n * sizeof(int)));
    nodeindex = static_cast<int *>(malloc(n * sizeof(int)));
    neighbors = static_cast<int **>(malloc(n * sizeof(int *)));
    memset(degrees, 0, n * sizeof(int));
    memset(nodeindex, 0, n * sizeof(int));

    for (long long j = 0; j < e; j++) {
        degrees[xs[j]]++;
        degrees[ys[j]]++;
    }

    for (int i = 0; i < n; i++)
        neighbors[i] = static_cast<int *>(malloc(degrees[i] * sizeof(int)));

    for (long long j = 0; j < e; j++) {
        neighbors[xs[j]][nodeindex[xs[j]]++] = ys[j];
        neighbors[ys[j]][nodeindex[ys[j]]++] = xs[j];
    }

    for (int i = 0; i < n; i++) {
        nodeindex[i] = 0;
        if (degrees[i] == 0)
            continue;
        std::sort(neighbors[i], neighbors[i] + degrees[i]);
        nodeindex[i] = 1;
        for (int j = 1; j < degrees[i]; j++)
            if (neighbors[i][j] != neighbors[i][j - 1])
                nodeindex[i]++;
    }

    e = 0;
    for (int i = 0; i < n; i++)
        e += nodeindex[i];

    fwrite(&n, sizeof(long long), 1, ot);
    fwrite(&e, sizeof(long long), 1, ot);

    long long x2 = 0;
    for (int i = 0; i < n; i++)
    {
        fwrite(&x2, sizeof(long long), 1, ot);
        x2 += nodeindex[i];
    }
    for (int i = 0; i < n; i++)
    {
        if (degrees[i] == 0)
            continue;
        fwrite(neighbors[i], sizeof(int), 1, ot);
        for (int j = 1; j < degrees[i]; j++)
            if (neighbors[i][j] != neighbors[i][j - 1])
                fwrite(&neighbors[i][j], sizeof(int), 1, ot);
    }
    srand((unsigned)time(NULL)); 
    if (rand_hetro) {
        int *type = new int[n];
        for (long long i = 0; i < e; i++) {
            type[i] = rand() % 5;
        }
        fwrite(type, sizeof(int), n, ot);
    } else if (hetro) {
        int *type = new int[n];
        FILE *hetro_file = fopen(hetro_string.c_str(), "r");
        memset(hetro_file, 0, sizeof(int) * n);
        int node, w;
        for (int i = 0; i < n; i++)
            fscanf(hetro_file, "%d %d", &node, &w);
        type[node] = w;
        fwrite(type, sizeof(int), n, ot);
    }
    
    if (rand_weight) {
        float *weight = new float[e];
        
        for (long long i = 0; i < e; i++) {
            weight[i] = rand() / float(RAND_MAX);
        }
        fwrite(weight, sizeof(float), e, ot);
    }
    
    fclose(ot);
}

int main(int argc, char **argv) {
    FILE *inputFile = nullptr;
    FILE *outputFile = nullptr;
    int a = 0;
    if ((a = argPos(const_cast<char *>("-input"), argc, argv)) > 0) {
        inputFile = fopen(argv[a + 1], "r");
        if (!inputFile) {
            std::cout << "Input file error" << std::endl;
        }
    } else {
        std::cout << "Missing input file" << std::endl;
    }
    if ((a = argPos(const_cast<char *>("-output"), argc, argv)) > 0) {
        outputFile = fopen(argv[a + 1], "wb");
    } else {
        std::cout << "Missing output file" << std::endl;
    }
    if ((a = argPos(const_cast<char *>("-weighted"), argc, argv)) > 0)
        weighted = true;
    if ((a = argPos(const_cast<char *>("-rand-w"), argc, argv)) > 0)
        rand_weight = true;

    if ((a = argPos(const_cast<char *>("-hetro"), argc, argv)) > 0)
        hetro = true;
    
    if ((a = argPos(const_cast<char *>("-node-type"), argc, argv)) > 0) {
        hetro_string = std::string(argv[a + 1]);
    } else if (hetro) {
        rand_hetro = 1;
    }
    work(inputFile, outputFile);
}