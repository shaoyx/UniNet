.PHONY:clean
CC = g++

vpath %.h include
vpath %.cpp src
vpath %.c src
vpath %.cpp src/models

CFLAGS = -g -fopenmp -I include -lpthread -std=c++11 -Ofast
WORD2VECFLAG = -lm -pthread -Ofast -march=native -Wall -funroll-loops -Wno-unused-result
OBJ = obj/train.o obj/main.o obj/edge2vec.o obj/deepwalk.o     \
	obj/fairwalk.o obj/node2vec.o obj/metapath.o obj/kgraph.o  \
	obj/walker.o obj/rw.o  obj/utils.o  obj/word2vec.o obj/sampler.o

all: uninet gen

uninet:$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
obj/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
obj/word2vec.o: word2vec.c
	gcc $(WORD2VECFLAG) -c $< -o $@
gen: src/gen.cpp
	$(CC) $< -o gen
clean:
	rm -f obj/*.o uninet gen

