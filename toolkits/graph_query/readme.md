**说明**：

1、支持的算法，无向无权图的最短路径距离索引算法 有向有权图的最宽路径距离索引算法  有向无权图的可达性索引算法，输入参数分别是 1 2 3

2、每种支持5种功能：normalquery normalbenchmark constructindex queryindex pllbenchmark

normalquery ：常规方法单次查询 后面1个参数：图文件路径

normalbenchmark ：常规方法benchmark，1000次的平均查询时间测试  后面1个参数：图文件路径

constructindex ：本系统剪枝索引算法的构建索引 后面2个参数：图文件路径 索引文件路径

queryindex ：剪枝索引算法的查询  后面1个参数：索引文件路径

pllbenchmark：剪枝索引算法的100w次的平均查询时间测试 后面1个参数：索引文件路径

3、特例

可达性算法（输入参数3）时，constructindex queryindex pllbenchmark 命令后要跟3个参数，分别是

图文件路径 scc文件路径 索引文件路径

**测试环境**：

g++编译器： g++7.3.0 均测试过可行

服务器：Linux

**使用方法**：

$ make   //用来build程序
$ bin/main  1    normalquery   graph.txt   //1代表支持的最短路径距离索引算法 
$ bin/main  2    constructindex graph.txt  graph.idx  //2代表支持的最宽路径距离索引算法 ，constructindex 表示构建索引功能



现在我开的shell直接是root权限连的大内存服务器 /public/disk/graph/下是所有工作代码，code/graph_query是你们源码，但编译失败，Index下是图文件和三个算法成功生成的索引文件