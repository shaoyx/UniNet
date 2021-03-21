# UniNet

UniNet is a scalable system for random-walk based network representation learning (NRL) and supports to learn node embedding over billion-edge networks.
It applies an efficient edge sampler based on Metropolis-Hastings sampling technique.
With the help of the new edge sampler, it unifies random walk based NRL models by exposing two APIs.

In UniNet, we have implemented five NRL models, [deepwalk](https://dl.acm.org/doi/10.1145/2623330.2623732), [node2vec](https://dl.acm.org/doi/10.1145/2939672.2939754), [metapath2vec](https://dl.acm.org/doi/10.1145/3097983.3098036), [edge2vec](https://bmcbioinformatics.biomedcentral.com/articles/10.1186/s12859-019-2914-2), and [fairwalk](https://www.ijcai.org/Proceedings/2019/456).

## Usage

First clone the repo with
```shell
git clone https://github.com/shaoyx/UniNet.git
```
Then compile with
```shell
cd UniNet
make
```
The above process generates 2 executable files, namely `uninet` and `gen`, where the latter is used for dataset pre-processing.

### Pre-processing

UniNet accepts CSR formatted network as input. The toolkit `gen` can convert text based edgelist network to CSR formatted network in binary. The format of the input edge list file is as follows, where one line corresponds to one edge.

```
5988 2048
5988 1542
5988 5138
5988 3607
5988 4125
...
```

**Example**
```shell
./gen -input data/blogcatalog_edge.txt -output data/blogcatalog.bin
```


**Full Options**
```
    -weighted      Generate network with edge weights.
    -rand-w        Assign random weight for edges in range
                    (0, 1).
    -hetro         Generate heterogeneous network file. 
                    If `--node-type` is not provided, assign 
                    random node types in range [1, 5].
    -node-type     File containing node type information.
```

### Quick-Start
We use deepwalk as an example.
```shell
./uninet -train -deepwalk -input data/blogcatalog.bin -output blogcatalog.emb
```
According to the above command, UniNet executes end-to-end training process and generates vector representation for network nodes. The output embedding file is formatted as the default setting for `gensim.word2vec`.


**General Settings**
* `-train` Executes the training process for generating embedding, otherwise only executes random walk process.
* `-input` Input CSR formatted network dataset.
* `-output` The output embedding file.
* `-out` Output the random walk trace.
* `-threads` Number of threads used for execution. The default is 1.
* `-walks` Number of walks starting from a single node. The default is 10.
* `-length` The length of a random walk. The default is 80.
* `-random`, `-burnin`, `-weight` Specify the initialization method of the Metropolis-Hastings based sampler. The default is 'random'.
* `-deepwalk`, `-node2vec`, `-metapath`, `-edge2vec`, `-fairwalk` Choose the model for execution. It must be noted that metapath2vec, edge2vec, and fairwalk must operate on networks with heterogeneous information.

**Model-Specific Options**
* `-p`, `-q` Parameters for node2vec, edge2vec, and fairwalk for the second-order random walk constrain.
* `-meta` Specify the metapath used for metapath2vec with a string of integers, `01210` for example. Note that the node type number must be within the limits of the network dataset, and the string must be circular, that is, the beginning and the end must be consistent.

**Embedding Training Options**
* `-size` The demension of embedding space. The default is 128.
* `-cbow` Whether to use cbow (If not, uses skip-gram). The default is 0.
* `-window` Word2vec skip window size. The default is 10.
* `-sample` Sub-sampling size. The default is 1e-3.
* `-negative` Negative sampling size for skip-gram. The default is 5.
* `-iter` Training iteration. The default is 1.

## Evaluation
The evaluation is conducted on a server with 24-core Xeon CPU and 96GB of memory. The parallelism is set to 16.

### Random Walk Generating

| Time(s) | **Deepwalk** | **Node2vec** | **Metapath2vec** | **Edge2vec** | **Fairwalk** |
|-|--------------|--------------|------------------|-------------|--------------|
|BlogCatalog| 0.07 |0.20 |0.16 |0.29 |0.33 |
|Amazon     | 1.83 |4.71 |3.97 |5.91 |6.31 |
|YouTube     | 13.16 |31.34 |22.33 |38.23 |43.11 |


### Embedding Training
| Dataset | BlogCatalog | Amazon | YouTube |
| ------- | ----------- | ------ | ------- |
|Traininng Time(s) |10.03 |437.13 |1512.99 |

### Accuracy Evaluation
The accuracy of the embedding is evaluated on BlogCatalog with multi-label node classification task. After the node embedding is acquired, we input the embedding into a classifier and utilize node labels of different proportions as training data to test the accuracy of the inference of remaining node labels.

| Train Ratio | 0.1 | 0.2 | 0.3 | 0.4 | 0.5 | 0.6 | 0.7 | 0.8 | 0.9 |
| ----------- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
|   Micro-F1  | 0.346 | 0.364 | 0.375 | 0.379 |0.379 |0.382 | 0.396 |0.401 |0.399 |
|   Macro-F1  |0.181 |0.213 |0.227 |0.238 |0.245 |0.253 |0.257 |0.254 | 0.256|

## Define New Models
The definition of the new model is implemented by inheriting `RWModel` class. Three interfaces must be implemented. 
Note that the state is defined as a pair of two integers, where the first represents the current node index, and the second contains the extra information.

**Dynamic Weight Definition**
```c++
virtual float computeWeight(
    State curState, EdgeIndexType nextEdgeIndex);
```
In this interface, the user needs to define the way to calculate the edge weight based on the state and the next edge. 

**State Transition**
```c++
virtual State newState(
    State curState, EdgeIndexType nextEdgeIndex);
```
The transition of states is crucial for the system, and must be implemented. We take the same state information as the last interface as the input, and calculates the next state.

**State Capacity**
```c++
virtual int stateNum(int vertex);
```
In order to determine the memory space allocated for the samplers for each node, the user needs to explicitly specify the number of states corresponding to each node, the result is returned as an integer.

After creating the model class, we need to integrate the model into the system by adding the model to `rw.cpp` in `RandomWalk::init()` and add a new command line argument. Take node2vec as an example, the interfaces are implemented as below.

```c++
float Node2vec::computeWeight(State curState, long long nextEdgeIndex) {
    long long curEdge = this->offsets[curState.first] + curState.second;
    int src = edges[curEdge];
    int nextV = edges[nextEdgeIndex];
    float nextW = weights[nextEdgeIndex];
    if (src == nextV) {
        return nextW / paramP;
    } else if (graph->has_edge(src, nextV)) {
        return nextW;
    } else {
        return nextW / paramQ;
    }
}

State Node2vec::newState(State curState, long long nextEdgeIndex) {
    int nextV = edges[nextEdgeIndex];
    int revOffset = edges_r[nextEdgeIndex] - offsets[nextV];
    State nextState;
    nextState.first = nextV;
    nextState.second = revOffset;
    return nextState;
}

int Node2vec::stateNum(int vertex) {
    return this->degrees[vertex];
}
```

Also, the model must be integrated into the system by adding the following code into `RandomWalk::init()`.
```c++
...
Node2vec *node2vec = new Node2vec(graph, argc, argv);
model = (RWModel *)node2vec;
...
```


## Citation

This project is licensed under the terms of [MIT](https://github.com/shaoyx/UniNet/blob/master/LICENSE) license. If the code is used, please cite as the following.
```
@inproceedings{Yao2020UniNetSN,
author = {Xingyu Yao and Yingxia Shao and Bin Cui and Lei Chen},
title = {UniNet: Scalable Network Representation Learning with Metropolis-Hastings Sampling},
year = {2021},
booktitle = {Proceedings of the 37th IEEE International Conference on Data Engineering},
series = {ICDE '21}
}
```
