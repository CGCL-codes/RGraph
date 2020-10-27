#ifndef N_2_HOP_LABELS_H
#define N_2_HOP_LABELS_H
/**
 * 支持有向无权图的输入，可达性查询，可达返回1，否则返回0
 */
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <vector>
#include <stack>
#include <map>
#include <queue>
#include <algorithm>
#include <set>
#include <string>
#include <time.h>
#include <iterator>
#include <regex>
using namespace std;
class Reachability{
private:
    struct Edge {
        int headVex, tailVex; //有向边的首(尾)结点编号
        Edge *headLink, *tailLink; //和当前边首(尾)结点相同的下一条边的指针

        Edge(int headVex, int tailVex) {
            this->headVex = headVex;
            this->tailVex = tailVex;
            this->headLink = this->tailLink = NULL;
        }
    };

    struct Node {
        bool tarjan; //在tarjan算法中是否已访问的标记
        bool bfs; //是否已经从当前结点搜索过
        bool reverse_bfs; //是否已经从当前结点逆向搜索过
        bool query; //是否查询过该点(避免标签中重复添加)
        int data; //结点的数据
        long long outindegree; //结点的入度+1乘出度+1
        vector<int> inNodes, outNodes; //可达当前结点的结点向量(当前结点的可达结点向量)
        Edge *firIn, *firOut; //结点的第一条入(出)边

        Node(int data) {
            this->data = data;
            this->tarjan = this->bfs = this->reverse_bfs = this->query = false;
            this->firIn = this->firOut = NULL;
        }
    };

    struct Graph {
        int nodeNum, edgeNum; //结点数和边数
        map<string, int> numberMap; //结点名称和结点编号的映射
        map<int, string> nameMap; //结点编号和结点名称的映射
        map<int, Node*> nodes; //结点编号和结点指针的映射
    };
    typedef pair<int, Node*> PAIR;

    Graph graph;                //原图数据
    vector<vector<int> > scc;  //强连通分量

    inline void insert_edge(Node* node, Edge* edge) //把边edge插入到以node为首结点的边链表中
    {
        Edge *last_edge = node->firOut;
        node->firOut = edge;
        edge->headLink = last_edge;
    }

    inline void insert_reverse_edge(Node* node, Edge* edge) //把边edge插入到以node为尾结点的边链表中
    {
        Edge *last_edge = node->firIn;
        node->firIn = edge;
        edge->tailLink = last_edge;
    }

    inline void delete_edge(Node* node, Edge* edge) //从node为首结点的边链表中把边edge删除
    {
        if (node->firOut == edge) {
            node->firOut = node->firOut->headLink;
        }
        else {
            for (Edge *tempEdge = node->firOut; tempEdge->headLink != NULL; tempEdge = tempEdge->headLink) {
                if (tempEdge->headLink == edge) {
                    tempEdge->headLink = tempEdge->headLink->headLink;
                    break;
                }
            }
        }
    }

    inline void delete_reverse_edge(Node* node, Edge* edge) //从node为尾结点的边链表中把边edge删除
    {
        if (node->firIn == edge) {
            node->firIn = node->firIn->tailLink;
        }
        else {
            for (Edge *tempEdge = node->firIn; tempEdge->tailLink != NULL; tempEdge = tempEdge->tailLink) {
                if (tempEdge->tailLink == edge) {
                    tempEdge->tailLink = tempEdge->tailLink->tailLink;
                    break;
                }
            }
        }
    }

    inline Node* init_inexistent_node(int nodeNum) //根据结点编号初始化一个结点(不存在)或返回结点的指针(存在)
    {
        if (graph.nodes[nodeNum] == NULL) {
            graph.nodes[nodeNum] = new Node(nodeNum);
        }

        return graph.nodes[nodeNum];
    }

    static bool cmp_by_degree(const PAIR& lhs, const PAIR& rhs) //按出入度的成绩排序
    {
        return lhs.second->outindegree > rhs.second->outindegree;
    }

    bool have_intersection(vector<int>& out_vec, vector<int>& in_vec) //有无交集
    {
        for (vector<int>::iterator out0 = out_vec.begin(), in0 = in_vec.begin(); (out0 != out_vec.end()) && (in0 != in_vec.end());) {
            if (*out0 == *in0)
                return true;
            else if (*out0 > *in0)
                ++in0;
            else
                ++out0;
        }
        return false;
    }

    bool binary_search(vector<int>& base, int key) //二分查找
    {
        int low = 0, high = base.size() - 1;
        while (low <= high) {
            int mid = (low + high) / 2;
            int pmid = base[mid];
            if (pmid == key) {
                return true;
            }
            else if (pmid > key) {
                high = mid - 1;
            }
            else {
                low = mid + 1;
            }
        }
        return false;
    }

public:
    int getVerticesNum(){
        return graph.nodeNum;
    }
    bool LoadIndex( const char *sccFile, const char *labelFile){
        ifstream sccin(sccFile);
        ifstream labelin(labelFile);
        if(!sccin || !labelin) return false;
        //将sccFile文件读入scc中
        vector<int> temp_line;
        string line;
        regex pat_regex("[[:digit:]]+");  //匹配原则，这里代表一个或多个数字

        while(getline(sccin, line)) {  //按行读取
            for (sregex_iterator it(line.begin(), line.end(), pat_regex), end_it; it != end_it; ++it) {  //表达式匹配，匹配一行中所有满足条件的字符
                temp_line.push_back(graph.numberMap[it->str()]);  //将数据转化为int型并存入一维vector中
            }
            scc.push_back(temp_line);  //保存所有数据
            temp_line.clear();
        }

        cout << "Combining scc..." << endl; //将每个强联通分量看成一个点
        combine_scc_node(scc);

        cout << "labelFile" << endl;
        //读入labelFile至labelVector中
        vector<vector<int > > labelVector;
        while(getline(labelin, line)) {  //按行读取
            for (sregex_iterator it(line.begin(), line.end(), pat_regex), end_it; it != end_it; ++it) {  //表达式匹配，匹配一行中所有满足条件的字符
                temp_line.push_back(stoi(it->str()));  //将数据转化为int型并存入一维vector中
            }
            labelVector.push_back(temp_line);  //保存所有数据
            temp_line.clear();
        }
        //读入labelVector至graph中的Node中
        for(int i = 0; i < labelVector.size(); i++){
            //注意label文件保存的都是结点名称，要转化为结点ID
            //先读取第一行，强联通分量的第一个点(代表点)
            int vID = graph.numberMap[to_string(labelVector[i][0])];
            //读取第二行的数据
            i++;
            int num = labelVector[i].size();
            //还原结点vID的出方向可达信息
            Node *node = graph.nodes[vID];
            for(int m = 0; m < num; m++){
                int uID = graph.numberMap[to_string(labelVector[i][m])];
                node->outNodes.push_back(uID);
            }
            //读取第三行数据
            i++;
            num = labelVector[i].size();
            //还原结点vID的入方向可达信息
            for(int m = 0; m < num; m++){
                int uID = graph.numberMap[to_string(labelVector[i][m])];
                node->inNodes.push_back(uID);
            }
        }

        return true;
    }
//输入是scc和保存了labelout的graph结构体变量，保存索引至scc和label文件中
    bool StoreIndex( const char *sccFile, const char *labelFile){
        ofstream sccout(sccFile);
        ofstream labelout(labelFile);
        if(!sccout || !labelout) return false;
        for (auto &s: scc) {
            for (auto &i : s) {
                sccout << graph.nameMap[i] << " ";
            }
            sccout << endl;
        } //输出每一个强联通分量到文件scc-out
        //graph中有labelout的索引信息，遍历保存
        for (auto beg = graph.nodes.begin(); beg != graph.nodes.end(); ++beg) { //对于图中的每一个点
            if (beg->first == beg->second->data) { //是强联通分量的第一个点(代表点)
                labelout << graph.nameMap[beg->first] << " " << endl; //输出结点的名称到文件labelFile
                for (auto i : beg->second->outNodes) {
                    labelout << graph.nameMap[i] << " "; //输出当前结点的可达结点的名称到文件labelFile
                }
                labelout << endl;
                for (auto i : beg->second->inNodes) {
                    labelout << graph.nameMap[i] << " "; //输出可达当前结点的结点的名称到文件labelFile
                }
                labelout << endl;
            }
        }
        return true;
    }

    bool ContructIndex(const char *filename){
        if(!LoadGraph(filename)) return false;
        cout << "Finding scc..." << endl; //分割强联通分量

        clock_t start = clock(); //计时开始

        map<int, int> def; //结点编号和def值的映射
        map<int, int> low; //结点编号和low值的映射
        map<int, bool> stack_sign; //结点编号和是否在栈中的映射
        stack<int> st;

        for (map<int, Node*>::iterator beg = graph.nodes.begin(); beg != graph.nodes.end(); ++beg) { //对于图中的每一个点
            if (!beg->second->tarjan) { //还未在tarjan算法中访问过
                tarjan(beg->second, def, low, st, scc, stack_sign); //执行tarjan算法
            }
        }
        cout << "Combining scc..." << endl; //将每个强联通分量看成一个点
        combine_scc_node(scc);

        clock_t finish = clock();

        cout << "Scc time: " << (finish - start) / CLOCKS_PER_SEC << " seconds" << endl; //输出处理强联通分量的时间

        cout << "Sorting by indegree and outdegree..." << endl; //按入度出度的乘积排序
        vector<PAIR> nodes_vec;

        for (map<int, Node*>::iterator beg = graph.nodes.begin(); beg != graph.nodes.end(); ++beg) {
            if (beg->first == beg->second->data) {
                long long outdegree = 1, indegree = 1;
                for (Edge *edge = beg->second->firOut; edge != NULL; edge = edge->headLink) {
                    ++outdegree;
                }
                for (Edge *edge = beg->second->firIn; edge != NULL; edge = edge->tailLink) {
                    ++indegree;
                }
                beg->second->outindegree = outdegree*indegree;
                nodes_vec.push_back(*beg);
            }
        }
        sort(nodes_vec.begin(), nodes_vec.end(), cmp_by_degree);

        cout << "Building 2-hops-label's data structure..." << endl; //通过搜索和逆向搜索建立2-hops-label

        start = clock(); //计时开始

        for (vector<PAIR>::iterator beg = nodes_vec.begin(); beg != nodes_vec.end(); ++beg) { //对于图中的每一个点
            search_out_node(beg->second); //搜索
            search_in_node(beg->second); //逆向搜索
        }

        finish = clock(); //计时结束

        cout << "Index time: " << (finish - start) / CLOCKS_PER_SEC << " seconds" << endl; //输出预处理时间

//        //将scc和graph保存的label信息可持久化
//        if(!StoreIndex(scc, graph, sccFile, labelFile)) return false;
//        cout << "Index store sucess!" << endl;
        return true;
    }

//读入图数据至结构体变量graph
    bool LoadGraph(const char *filename){
        cout << "Loading data from file " << filename << "..." << endl; //读入数据

        //统计图的结点数和边数
        int nodeNum = 0, edgeNum = 0;
        ifstream ifs(filename);
        if(!ifs) return false;
        for (int v, w; ifs >> v >> w; ) {
            edgeNum++;
            nodeNum = max(nodeNum, max(v, w) + 1);
        }
        ifs.close();
        graph.nodeNum = nodeNum;
        graph.edgeNum = edgeNum;
        cout << "graph.nodeNum:" << graph.nodeNum << "graph.edgeNum:" << graph.edgeNum << endl;

        ifstream fin(filename);
        if(!fin) return false;
//        fin >> graph.nodeNum >> graph.edgeNum; //读入结点数和边数
        int numberCount = 0;
        string inNodeID, outNodeID;
        int inNodeNum, outNodeNum;
        Node *inNode = NULL, *outNode = NULL;
        map<int, int> edgesMap;

        for (int j = 0; j < graph.edgeNum; j++) {
            fin >> outNodeID >>  inNodeID; //读入结点名称
            if (outNodeID == inNodeID) { //出现自环则删掉这条边，对可达性无影响
                continue;
            }
            if (graph.numberMap[outNodeID] == 0) {
                outNodeNum = graph.numberMap[outNodeID] = ++numberCount;
                graph.nameMap[outNodeNum] = outNodeID;
            }
            else {
                outNodeNum = graph.numberMap[outNodeID];
            } //建立结点名称和结点编号的满射

            if (graph.numberMap[inNodeID] == 0) {
                inNodeNum = graph.numberMap[inNodeID] = ++numberCount;
                graph.nameMap[inNodeNum] = inNodeID;
            }
            else {
                inNodeNum = graph.numberMap[inNodeID];
            } //建立结点名称和结点编号的满射

            if (edgesMap[outNodeNum] != inNodeNum) { //出现重边则删掉这条边，对可达性无影响
                outNode = init_inexistent_node(outNodeNum);
                inNode = init_inexistent_node(inNodeNum); //根据结点编号得到结点的指针
                Edge *edge = new Edge(outNodeNum, inNodeNum); //新建一条边edge
                insert_edge(outNode, edge); //把边edge插入到以outNode为首结点的边链表中
                insert_reverse_edge(inNode, edge); //把边edge插入到以inNode为尾结点的边链表中
                edgesMap[outNodeNum] = inNodeNum; //记录下这条边
            }
        }
        return true;
    }


    void tarjan(Node *node, map<int, int> &def, map<int, int> &low, stack<int> &st,
                vector<vector<int> > &scc, map<int, bool> &stack_sign) //分割强连通分量
    {
        if (node == NULL) {
            cout << "In tarjan function: the variable 'node' is nullptr" << endl;
            exit(1);
        }

        static int index = 0;
        def[node->data] = low[node->data] = ++index; //初始化当前结点的def和low的值
        st.push(node->data); //遍历到的结点入栈
        stack_sign[node->data] = true; //标记为在栈中
        node->tarjan = true; //标记为已访问

        Edge *edge = node->firOut; //当前结点的第一条出边
        while (edge != NULL) {
            Node *next_node = graph.nodes[edge->tailVex]; //根据结点编号找到指向该结点的指针
            if (!next_node->tarjan) { //该结点未访问
                tarjan(next_node, def, low, st, scc, stack_sign);
                low[node->data] = min(low[node->data], low[next_node->data]);
            }
            else if (stack_sign[next_node->data]) { //该结点已访问且在栈中
                low[node->data] = min(low[node->data], def[next_node->data]);
            }
            edge = edge->headLink; //和当前边的首结点相同的下一条边
        }

        if (def[node->data] == low[node->data]) { //def和low的值相等，发现新的强连通分量
            vector<int> new_scc;
            int top_data;
            do {
                top_data = st.top();
                stack_sign[top_data] = false; //标记为不在栈中
                new_scc.push_back(top_data);
                st.pop();
            } while (top_data != node->data);
            scc.push_back(new_scc);
        }
    }

    void combine_scc_node(vector<vector<int> > &scc) //将每个强联通分量看成一个点
    {
        for (vector<vector<int> >::iterator s = scc.begin(); s != scc.end(); s++) { //对于每一个强联通分量
            if ((*s).size() < 2) { //当前强连通分量只有一个点时不做处理
                continue;
            }
            for (vector<int>::iterator beg = (*s).begin(); beg != (*s).end(); ++beg) { //对于当前强联通分量中的每一个点
                graph.nodes[*beg]->data = *((*s).begin());
            }
        }
        for (vector<vector<int> >::iterator s = scc.begin(); s != scc.end(); s++) { //对于每一个强联通分量
            if ((*s).size() < 2) { //当前强连通分量只有一个点时不做处理
                continue;
            }
            int first = *((*s).begin()); //当前强连通分量的第一个点
            map<int, bool> scc_sign; //是否属于当前强联通分量
            map<int, bool> accessed; //是否已访问
            for (vector<int>::iterator i = (*s).begin(); i != (*s).end(); i++) //对于当前强联通分量中的每一个点
                scc_sign[*i] = true; //标记为属于当前强联通分量
            for (vector<int>::iterator beg = (*s).begin(); beg != (*s).end(); ++beg) { //对于当前强联通分量中的每一个点
                int r = *beg;
                if (beg == (*s).begin()) { //当前结点是强连通分量的第一个点时
                    for (Edge *edge = graph.nodes[r]->firOut; edge != NULL; edge = edge->headLink) {
                        accessed[graph.nodes[edge->tailVex]->data] = true;
                    }
                    for (Edge *edge = graph.nodes[r]->firIn; edge != NULL; edge = edge->tailLink) {
                        accessed[graph.nodes[edge->headVex]->data] = true;
                    }
                    continue;
                }
                for (Edge *edge = graph.nodes[r]->firOut; edge != NULL;) { //对于当前结点的每一条出边
                    Edge *temp = edge; //temp表示当前边
                    edge = edge->headLink; //edge表示下一条边
                    if (!scc_sign[temp->tailVex] && !accessed[graph.nodes[temp->tailVex]->data]) { //当前边的尾结点不属于强连通分量还未访问
                        delete_edge(graph.nodes[r], temp);
                        temp->headVex = first; //当前边的首结点设为强联通分量的第一个点
                        insert_edge(graph.nodes[first], temp); //把当前边插入到以第一个点为首结点的边链表中
                        accessed[graph.nodes[temp->tailVex]->data] = true; //当前边的尾结点标记为已访问
                    }
                    else { //当前边的尾结点属于强连通分量或者已访问
                        delete_edge(graph.nodes[r], temp);
                        delete_reverse_edge(graph.nodes[temp->tailVex], temp);
                        delete temp; //删除这条边
                    }
                }
                for (Edge *edge = graph.nodes[r]->firIn; edge != NULL;) { //对于当前结点的每一条入边
                    Edge *temp = edge; //temp表示当前边
                    edge = edge->tailLink; //edge表示下一条边
                    if (!scc_sign[temp->headVex] && !accessed[graph.nodes[temp->headVex]->data]) { //当前边的首结点不属于强连通分量还未访问
                        delete_reverse_edge(graph.nodes[r], temp);
                        temp->tailVex = first; //当前边的尾结点设为强联通分量的第一个点
                        insert_reverse_edge(graph.nodes[first], temp); //把当前边插入到以第一个点为尾结点的边链表中
                        accessed[graph.nodes[temp->headVex]->data] = true; //当前边的首结点标记为已访问
                    }
                    else { //当前边的首结点属于强连通分量或者已访问
                        delete_reverse_edge(graph.nodes[r], temp);
                        delete_edge(graph.nodes[temp->headVex], temp);
                        delete temp; //删除这条边
                    }
                }
            }
        }
    }

    void search_out_node(Node *node) //搜索
    {
        queue<int> q; //搜索队列
        map<int, bool> accessed; //结点是否访问过
        Node *cur_node = node; //当前结点设为初始结点

        for (Edge *edge = cur_node->firOut; edge != NULL; edge = edge->headLink) //对于当前结点的每一条出边
            q.push(edge->tailVex); //当前边的尾结点入队

        while (!q.empty()) {
            cur_node = graph.nodes[q.front()]; //当前结点设为队首结点
            q.pop();

            if (node->data != cur_node->data && !(graph.nodes[cur_node->data]->bfs) && !query(node->data, cur_node->data)) {
                //初始结点和当前结点不在同一个强联通分量且还未从当前强连通分量搜索过且初始结点和当前结点查询结果为不可达
                for (Edge *edge = cur_node->firOut; edge != NULL; edge = edge->headLink) { //对于当前结点的每一条出边
                    if (!accessed[edge->tailVex]) { //当前边的尾结点未访问
                        q.push(edge->tailVex); //当前边的尾结点入队
                    }
                }

                if (!accessed[cur_node->data]) { //当前结点的强联通分量的第一个结点未访问
                    cur_node->inNodes.insert(upper_bound(cur_node->inNodes.begin(), cur_node->inNodes.end(), node->data), node->data);
                    //把初始结点的强联通分量的第一个结点插入可达当前结点的结点向量的恰当位置使之排好序
                    accessed[cur_node->data] = true; //标记当前结点的强联通分量的第一个结点为已访问
                }
            }
        }
        node->bfs = true; //标记为已经从初始结点搜索过
    }

    void search_in_node(Node *node) //逆向搜索
    {
        queue<int> q; //逆向搜索队列
        map<int, bool> accessed; //结点是否访问过
        Node *cur_node = node; //当前结点设为初始结点

        for (Edge *edge = cur_node->firIn; edge != NULL; edge = edge->tailLink) //对于当前结点的每一条入边
            q.push(edge->headVex);  //当前边的首结点入队


        while (!q.empty()) {
            cur_node = graph.nodes[q.front()];  //当前结点设为队首结点
            q.pop();

            if (node->data != cur_node->data && !(graph.nodes[cur_node->data]->reverse_bfs) && !query(cur_node->data, node->data)) {
                //初始结点和当前结点不在同一个强联通分量且还未从当前强连通分量逆向搜索过且当前结点和初始结点查询结果为不可达
                for (Edge *edge = cur_node->firIn; edge != NULL; edge = edge->tailLink) {  //对于当前结点的每一条入边
                    if (!accessed[edge->headVex]) { //当前边的首结点未访问
                        q.push(edge->headVex); //当前边的首结点入队
                    }
                }

                if (!accessed[cur_node->data]) { //当前结点的强联通分量的第一个结点未访问
                    cur_node->outNodes.insert(upper_bound(cur_node->outNodes.begin(), cur_node->outNodes.end(), node->data), node->data);
                    //把初始结点的强联通分量的第一个结点插入当前结点的可达结点向量的恰当位置使之排好序
                    accessed[cur_node->data] = true; //标记当前结点的强联通分量的第一个结点为已访问
                }
            }
        }
        node->reverse_bfs = true; //标记为已经从初始结点逆向搜索过
    }


    bool query2(int outNodeName, int inNodeName) //查询两个点是否可达
    {
        if (graph.numberMap.find(to_string(outNodeName)) == graph.numberMap.end()
                    || graph.numberMap.find(to_string(inNodeName)) == graph.numberMap.end()){
            return false;
        }
        int outNodeNum = graph.numberMap[to_string(outNodeName)];
        int inNodeNum = graph.numberMap[to_string(inNodeName)];
        int outNodeScc = graph.nodes[outNodeNum]->data; //起始结点的强联通分量的第一个结点
        int inNodeScc = graph.nodes[inNodeNum]->data; //终止结点的强联通分量的第一个结点

        Node *outNode = graph.nodes[outNodeScc];
        Node *inNode = graph.nodes[inNodeScc];

        return have_intersection(outNode->outNodes, inNode->inNodes) ||
               binary_search(inNode->inNodes, outNodeScc) ||
               binary_search(outNode->outNodes, inNodeScc) ||
               outNodeScc == inNodeScc; //有无交集代表是否可达
    }

    bool query(int outNodeNum, int inNodeNum) //查询两个点是否可达
    {
        int outNodeScc = graph.nodes[outNodeNum]->data; //起始结点的强联通分量的第一个结点
        int inNodeScc = graph.nodes[inNodeNum]->data; //终止结点的强联通分量的第一个结点

        Node *outNode = graph.nodes[outNodeScc];
        Node *inNode = graph.nodes[inNodeScc];

        return have_intersection(outNode->outNodes, inNode->inNodes) ||
               binary_search(inNode->inNodes, outNodeScc) ||
               binary_search(outNode->outNodes, inNodeScc) ||
               outNodeScc == inNodeScc; //有无交集代表是否可达
    }
};
#endif //N_2_HOP_LABELS_H