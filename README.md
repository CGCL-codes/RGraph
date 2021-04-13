# RGraph
RGraph is an RDMA-assisted asynchronous distributed graph processing system. RGraph distributes edges into two parts to isolate master and mirror vertices. RGraph exploits the asymmetry of RDMA to accelerate the one-to-many communication between master and mirror vertices. The results in comprehensive experiments show that compared to existing designs, PowerGraph, RGraph reduces the execution time by up to 81%.

## Introduction
Graphs can present connections between entities and thus are adopted in many scenarios. For example, Google organizes links between network pages as a graph and analyzes it to determine which pages are of most interest to users; Twitter denotes interconnections between users as a social graph and processes it for followee recommendation. To exploit the information inside a graph, graph processing has attracted increasingly attention. As real-world graphs become more and more large and complex, a single machine cannot support efficient graph processing. 

To efficiently perform iteration, some distributed graph processing systems use the synchronous execution model, which sets barriers between iterations. When a vertex finishes computation, it transfers the data to its neighbors. The computation of the next iteration relies on the results of the current iteration. Hence, the next iteration cannot start until the computation and communication of the current iteration on all vertices are completed. It is not hard to see that the completion of one iteration depends on the slowest vertex.

Recently, the emergence of Remote Direct Memory Access (RDMA) technology brings opportunities for distributed graph processing. Due to the ability of zero-copy and kernel-bypassing, the RDMA network can achieve high throughput and low latency with low CPU utilization. For example, the latest RDMA network can provide 200 Gbps throughput and 1 us latency. If we implement distributed graph processing systems on RDMA network, the performance of graph processing can be further improved.

Based on the asymmetry of RDMA, we propose RGraph, a novel RDMA-based distributed graph processing system that supports asynchronous execution. RGraph investigates the vertex communication pattern and designs a new graph partition method, which gathers master vertices on a small part of the nodes and spread slave vertices on other nodes. In this condition, in-bound RDMA communication and out-bound RDMA communication can be distributed separately on different parts of the nodes. RGraph further exploits RDMA’s asymmetry to greatly reduce the communication overhead. We implement RGraph on top of PowerGraph, a well-known distributed graph processing system, and conduct comprehensive experiments with large-scale real-world graphs to evaluate the design. The results demonstrate that RGraph substantially outperforms existing systems.

## Structure of RGraph
![image](https://github.com/RGraph/RGraph/blob/master/images/RGraphStructure.png)

Nodes in the cluster are connected by the RDMA network. When a graph processingjob is submitted, the system reads graph data from the local file system or a distributed file system (e.g., HDFS) and partitions the graph into multiple nodes. Every node runs an independent process, which manages the local graph partition and communicates with others. RGraph contains two processing modules: destination-aware edge assignment module and graph partitionmodule. The destination-aware edge assignment module divides all edges into two parts based on our proposed algorithm. One part of edges that can cover all vertices belong to the k nodes, while the other part of edges belong to the remaining p - k nodes. Once the two parts of edges are determined, the graph partition module performs different graph partitioning methods to further assign the two parts of edges with different objectives. When graph partitioning is finished, the system runs the specified algorithm with RDMA one-sided communication.

## How to use?

### Enviroment
We implement RGraph on a 20-node cluster. Each node in the cluster is equipped with a 16-core Intel 2.4GHz Xeon CPU, 64GB RAM, 256GB HDD, 1Gbps NIC, and a 56Gbps Mellanox InfiniBand NIC. The operating system is Red Hat Enterprise Linux Server release 6.2.

### Initial Setting
Configure the project and complie examples
```shell
$ ./configure --no_jvm
$ cd release/toolkits/graph_analytics
$ make -j4
```

Create a file, named machines, in the project source directory. Add the hostname of all computing machines into the file.
```shell
$ touch machines
$ vi machines
```

### Running Examples
Switch the directory to the example directroy and running examples.
```shell
$ cd release/toolkits/graph_analytics
$ mpiexec -n machineNumber -hostfile pathToMachines release/toolkits/graph_analytics/pagerank --format=snap --graph=graphData
```


## Publications
If you want to know more detailed information, please refer to this paper:

Hanhua Chen, Jie Yuan, Hai Jin, Yonghui Wang, Sijie Wu, Zhihao Jiang. "RGraph: Asynchronous Graph Processing Based on Asymmetry of RDMA." Software Practice and Experience (SPE), 2021.

## Authors and Copyright
RGraph is developed in National Engineering Research Center for Big Data Technology and System, Cluster and Grid Computing Lab, Services Computing Technology and System Lab, School of Computer Science and Technology, Huazhong University of Science and Technology, Wuhan, China by Hanhua Chen ([chen@hust.edu.cn](chen@hust.edu.cn)), Jie Yuan ([jieyuan@hust.edu.cn](jieyuan@hust.edu.cn)), Hai Jin ([hjin@hust.edu.cn](hjin@hust.edu.cn)), Yonghui Wang ([yhw@hust.edu.cn](yhw@hust.edu.cn)), Sijie Wu ([wsj@hust.edu.cn](wsj@hust.edu.cn)), Zhihao Jiang ([hustjiangzh@hust.edu.cn](hustjiangzh@hust.edu.cn)).

Copyright (C) 2021, [STCS & CGCL](http://grid.hust.edu.cn/) and [Huazhong University of Science and Technology](https://www.hust.edu.cn/).
