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

#include "utils.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

int sampleApp;

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

myrandom::myrandom(uint64_t seed) {
    for (int i = 0; i < 2; i++) {
    long long z = seed += UINT64_C(0x9E3779B97F4A7C15);
    z = (z ^ z >> 30) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ z >> 27) * UINT64_C(0x94D049BB133111EB);
    if (i == 0)
        rng_seed0 = z ^ (z >> 31);
    else
        rng_seed1 = z ^ (z >> 31);
    }
}

void myrandom::reinit(uint64_t seed) {
    for (int i = 0; i < 2; i++) {
        long long z = seed += UINT64_C(0x9E3779B97F4A7C15);
        z = (z ^ z >> 30) * UINT64_C(0xBF58476D1CE4E5B9);
        z = (z ^ z >> 27) * UINT64_C(0x94D049BB133111EB);
        if (i == 0)
            rng_seed0 = z ^ (z >> 31);
        else
            rng_seed1 = z ^ (z >> 31);
    }
}

uint64_t myrandom::lrand() {
    const uint64_t s0 = rng_seed0;
    uint64_t s1 = rng_seed1;
    const uint64_t result = s0 + s1;
    s1 ^= s0;
    rng_seed0 = rotl(s0, 55) ^ s1 ^ (s1 << 14);
    rng_seed1 = rotl(s1, 36);
    return result;
}

double myrandom::drand() {
    const union un {
        uint64_t i;
        double d;
    } a = {UINT64_C(0x3FF) << 52 | lrand() >> 12};
    return a.d - 1.0;
}

void init_sigmoid_table() { // this shoould be called before fast_sigmoid once
    sigmoid_table = static_cast<float *>(malloc((sigmoid_table_size + 1) * sizeof(float)));
    for (int k = 0; k != sigmoid_table_size; k++) {
        float x = 2 * SIGMOID_BOUND * k / sigmoid_table_size - SIGMOID_BOUND;
        sigmoid_table[k] = 1 / (1 + exp(-x));
    }
}

float fast_sigmoid(double x) {
    if (x > SIGMOID_BOUND)
        return 1;
    if (x < -SIGMOID_BOUND)
        return 0;
    int k = (x + SIGMOID_BOUND) * SIGMOID_RESOLUTION;
    return sigmoid_table[k];
}

bool eq(float a, float b) {
    return abs(a - b) < 1e-4;
}