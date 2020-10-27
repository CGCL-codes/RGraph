#ifndef SYSTEMFEATURES_H
#define SYSTEMFEATURES_H
/**
 * 系统每个算法提供五个功能
 */
#include "pruned_landmark_labeling.h"
#include "pllwidestpath.h"
#include "2-hop-labels.h"

using namespace std;
class SystemFeatures {
public:
    virtual void ConstructIndex(int argc, char **argv) = 0;
    virtual void QueryIndex(int argc, char **argv) = 0;
    virtual void PllBenchmark(int argc, char **argv) = 0;
    virtual void NormalQuery(int argc, char **argv) = 0;
    virtual void NormalBenchmark(int argc, char **argv) = 0;
};

class ShortestDistance:public SystemFeatures{
private:
    double GetCurrentTimeMicroSec() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1e6 + tv.tv_usec;
    }
    double GetCurrentTimeSec() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }
    bool LoadFile(const char *filename,vector<pair<uint32_t, uint32_t> > &es){
        ifstream ifs(filename);
        for (uint32_t v, w; ifs >> v >> w; ) {
            es.push_back(make_pair(v, w));
        }
        if (ifs.bad()) return false;
        return true;
    }

    //start:v   end:w
    uint32_t BFS(const vector<pair<uint32_t, uint32_t> > &es,uint32_t start,uint32_t end){
        if(start == end) return 0;
        uint32_t V = 0;
        for (size_t i = 0; i < es.size(); ++i) {
            V = max(V, max(es[i].first, es[i].second) + 1);
        }
        //若输入不符合规范，返回不可达
        if(start < 0 || start >= V || end < 0 || end >= V)  return INT32_MAX;
        vector<vector<uint32_t> > adj(V);
        for (size_t i = 0; i < es.size(); ++i) {
            uint32_t v = es[i].first, w = es[i].second;
            adj[v].push_back(w);
            adj[w].push_back(v);
        }
        queue<uint32_t> que;
        vector<uint32_t> distance(V,INT_MAX);
        que.push(start);
        distance[start] = 0;
        while(!que.empty()){
            uint32_t v = que.front();
            que.pop();
            for (size_t i = 0; i < adj[v].size(); ++i) {
                uint32_t w = adj[v][i];
                // 如果不等于INT_MAX，说明顶点w被访问过了
                if (distance[w] == INT_MAX) {
                    que.push(w);
                    distance[w] = distance[v] + 1;
                }
                if(w == end) {return distance[w];}
            }
        }
        return INT_MAX;
    }

public:
    void ConstructIndex(int argc, char **argv){
        if (argc != 3) {
            cerr << "usage: construct_index GRAPH INDEX" << endl;
            exit(EXIT_FAILURE);
        }

        PrunedLandmarkLabeling pll;
        if (!pll.ConstructIndex(argv[1])) {
            cerr << "error: Load or construction failed" << endl;
            exit(EXIT_FAILURE);
        }
        pll.PrintStatistics();

        if (!pll.StoreIndex(argv[2])) {
            cerr << "error: Store failed" << endl;
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    virtual void QueryIndex(int argc, char **argv){
        if (argc != 2) {
            cerr << "usage: construct_index INDEX" << endl;
            exit(EXIT_FAILURE);
        }

        PrunedLandmarkLabeling pll;
        cout << "Loading...Please Wait" << endl;
        if (!pll.LoadIndex(argv[1])) {
            cerr << "error: Load failed" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load Success!" << endl;
        cout << "Please input two vertices: (eg. 1 2)" << endl;
        int u,v;
        while(true){
            if(cin >> u >> v){
                double query_time = -GetCurrentTimeMicroSec();
                cout << "distance: " << pll.QueryDistance(u, v) << endl;
                query_time += GetCurrentTimeMicroSec();
                cout << "query_time:" << query_time << " us" << endl;
            }
            else{
                cout <<"Wrong fomat!"<<endl;
                cin.clear();
                cin.ignore();
            }
        }
    }
    void PllBenchmark(int argc, char **argv){
        if (argc != 2) {
            cerr << "usage: load_index  filename" << endl;
            exit(EXIT_FAILURE);
        }
        int kNumQueries = 1000000;
        cout << "Load file, waiting ... " << endl;
        PrunedLandmarkLabeling pll;
        if(!pll.LoadIndex(argv[1])){
            cerr << "error: Load failed" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load sucess ! " << endl;

        cout << "Querying randomly 1000000 times..." << endl;
        vector<int> vs(kNumQueries), ws(kNumQueries);
        for (int i = 0; i < kNumQueries; ++i) {
            vs[i] = rand() % pll.GetNumVertices();
            ws[i] = rand() % pll.GetNumVertices();
        }

        double time_start = GetCurrentTimeSec();
        for (int i = 0; i < kNumQueries; ++i) {
            pll.QueryDistance(vs[i], ws[i]);
        }
        double elapsed_time = GetCurrentTimeSec() - time_start;
        cout << "average query time: "
             << elapsed_time / kNumQueries * 1E6
             << " microseconds" << endl;

        exit(EXIT_SUCCESS);
    }
    void NormalQuery(int argc, char **argv){
        if(argc != 2){
            cerr << "usage:command file" << endl;
        }
        cout << "Load File..." << endl;
        vector<pair<uint32_t, uint32_t> > es;
        if(!LoadFile(argv[1],es)){cerr << "Loading failed!" << endl;return;}
        cout << "Load success!" << endl;

        int u,v;
        while(true){
            if(cin >> u >> v){
                double query_time = -GetCurrentTimeMicroSec();
                cout << "distance: " << BFS(es, u, v) << endl;
                query_time += GetCurrentTimeMicroSec();
                cout << "query_time:" << query_time * 1e-3 << "ms" << endl;
            }
            else{
                cout <<"Wrong fomat!"<<endl;
                cin.clear();
                cin.ignore();
            }
        }
    }
    void NormalBenchmark(int argc, char **argv){
        if(argc != 2){
            cerr << "usage:command file" << endl;
        }
        int kNumQueries = 100;
        cout << "Load File..." << endl;
        vector<pair<uint32_t, uint32_t> > es;
        if(!LoadFile(argv[1],es)){cerr << "Loading failed!" << endl;return;}
        cout << "Load success!" << endl;

        cout << "Querying randomly 100 times..." << endl;
        uint32_t V = 0;
        for (size_t i = 0; i < es.size(); ++i) {
            V = max(V, max(es[i].first, es[i].second) + 1);
        }

        vector<int> vs(kNumQueries), ws(kNumQueries);
        for (int i = 0; i < kNumQueries; ++i) {
            vs[i] = rand() % V;
            ws[i] = rand() % V;
        }

        double time_start = GetCurrentTimeSec();
        for (int i = 0; i < kNumQueries; ++i) {
            BFS(es, vs[i], ws[i]);
        }
        double elapsed_time = GetCurrentTimeSec() - time_start;
        cout << "average query time: "
             << elapsed_time / kNumQueries * 1E6
             << " microseconds" << endl;

        exit(EXIT_SUCCESS);
    }
};

#include <iostream>
#include <set>
#include <stack>
#include <vector>
#include <fstream>
#include <map>
#include <sys/time.h>

using namespace std;
#define INF 99999
typedef pair<float, int> iPair; // iPair的float保存的是边的权值

class Graph{
public:
    int V = 0,E = 0;
    vector<vector<iPair > > adj;  //保存图的邻接表
    vector<float> distTo;        //保存最宽路径距离
    vector<int > P;

    bool constructGraph(const char *filename);
    bool constructGraph(istream &ifs);
    void widsetPath(int src, int des);
    void showWidestPath(int des);
};
bool Graph::constructGraph(const char *filename) {
    ifstream ifs(filename);
    return ifs && constructGraph(ifs);
}
bool Graph::constructGraph(istream &ifs) {
    int u,v;
    float weight;
    map<int, vector<iPair > > tmp;
    for(;ifs >> u >> v >> weight;){
        V = max(u, max(V, v));
        E++;
        tmp[u].push_back(make_pair(weight, v));
    }
    V++;
    vector<vector<iPair > > adj_tmp(V);
    adj.swap(adj_tmp);
    //遍历tmp
    auto iter = tmp.begin();
    while(iter != tmp.end()){
        int k = iter->first;
        for(int i = 0; i < tmp[k].size(); i++){
            adj[k].push_back(tmp[k][i]);
//            cout << k << "->" << tmp[k][i].second << ": " << tmp[k][i].first << endl;
        }
        iter++;
    }


}
void Graph::widsetPath(int src, int des) {
    if(src < 0 || src >= V || des < 0 || des >= V) return;
    set< iPair, greater<iPair> > maxHeap;  //权值大的优先级高,最大堆
    //先将distTo初始化为-INF
    vector<float > tmp_arr(V, -INF);
    distTo.swap(tmp_arr);

    maxHeap.insert(make_pair(INF, src));
    distTo[src] = INF;

    // 保存路径
    vector<int > tmpPath(V, -1);
    P.swap(tmpPath);
    P[src] = src;

    //运行修改的Dijkstra算法求解最宽路径距离
    while(!maxHeap.empty()){
        iPair tmp = *(maxHeap.begin()); //取出set集合中权值最大的边
        maxHeap.erase(maxHeap.begin());
        int u = tmp.second;
//        if(u == des)
//            return;   //如果终点des已经在集合中，说明src -> des 的最宽距离已经求出，直接返回
        for(auto i = adj[u].begin(); i != adj[u].end(); i++){
            int v = (*i).second;
            float weight = (*i).first;
            float dis = distTo[u] > weight ? weight : distTo[u];
            if(distTo[v] < dis){
                if(distTo[v] != -INF){
                    //说明最大堆的v需要更新其value
                    maxHeap.erase(maxHeap.find(make_pair(distTo[v], v)));
                }
                distTo[v] = dis;
                maxHeap.insert(make_pair(distTo[v], v));
                P[v] = u;
            }
        }
    }
}
void Graph::showWidestPath(int des) {
    int tmp = des;
    stack<int> s;
    while(P[tmp] != tmp){
        if(P[tmp] == -1)
            return;;
        s.push(tmp);
        tmp = P[tmp];
    }
    s.push(tmp);
    while(!s.empty()){
        cout << s.top();
        s.pop();
        if(!s.empty()) cout << "->";
    }
    cout << endl;
//    for(int i = 0; i < V; i++){
//        cout << "P[" << i << "]=" << P[i] << endl;
//    }

}

class WidestDistance:public SystemFeatures{
private:
    double GetCurrentTimeMicroSec() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1e6 + tv.tv_usec;
    }
    double GetCurrentTimeSec() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }

public:
    void ConstructIndex(int argc, char **argv){
        if (argc != 3) {
            cerr << "usage: construct_index GRAPH INDEX" << endl;
            exit(EXIT_FAILURE);
        }
        PllWidestPath pll;
        if (!pll.constructIndex(argv[1])) {
            cerr << "error: Construction failed" << endl;
            exit(EXIT_FAILURE);
        }
        pll.printStatistic();
        if (!pll.storeIndex(argv[2])) {
            cerr << "error: Store failed" << endl;
            exit(EXIT_FAILURE);
        }
    }
    void QueryIndex(int argc, char **argv){
        if (argc != 2) {
            cerr << "usage: load_index  INDEX" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load file, waiting ... " << endl;
        PllWidestPath pll;
        if(!pll.loadIndex(argv[1])){
            cerr << "error: Load failed" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load sucess ! " << endl;


        uint32_t u,w;
        while(true){
            if(cin >> u >> w){
                double query_time = 0;
                query_time = -GetCurrentTimeMicroSec();
                cout << "distance of widest path: " << pll.queryDistance(u, w) << endl;
                query_time += GetCurrentTimeMicroSec();
                cout << "query_time:" << query_time << "us" << endl;
            }
            else{
                cout <<"Wrong fomat!"<<endl;
                cin.clear();
                cin.ignore();
            }
        }
    }
    void PllBenchmark(int argc, char **argv){
        if (argc != 2) {
            cerr << "usage: load_index  filename" << endl;
            exit(EXIT_FAILURE);
        }
        int kNumQueries = 1000000;
        cout << "Load file, waiting ... " << endl;
        PllWidestPath pll;
        if(!pll.loadIndex(argv[1])){
            cerr << "error: Load failed" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load sucess ! " << endl;

        cout << "Querying randomly 1000000 times..." << endl;
        vector<int> vs(kNumQueries), ws(kNumQueries);
        for (int i = 0; i < kNumQueries; ++i) {
            vs[i] = rand() % pll.getVerticeNum();
            ws[i] = rand() % pll.getVerticeNum();
        }

        double time_start = GetCurrentTimeSec();
        for (int i = 0; i < kNumQueries; ++i) {
            pll.queryDistance(vs[i], ws[i]);
        }
        double elapsed_time = GetCurrentTimeSec() - time_start;
        cout << "average query time: "
             << elapsed_time / kNumQueries * 1E6
             << " microseconds" << endl;

        exit(EXIT_SUCCESS);
    }
    void NormalQuery(int argc, char **argv){
        if (argc != 2) {
            cerr << "usage: normalWidestPathQuery GRAPH" << endl;
            exit(EXIT_FAILURE);
        }
        Graph *graph = new Graph();
        graph->constructGraph(argv[1]);
        cout << "graph loaded!" << endl;
        cout << "vertex num: " << graph->V  << ", edge num: " <<  graph->E  << endl;
        int s, t;
        while(true){
            if(cin >> s >> t){
                if(s > graph->V || t > graph->V || s < 0 || t < 0) continue;
                double query_time = -GetCurrentTimeMicroSec();
                graph->widsetPath(s, t);
                cout << "widest distance: " << graph->distTo[t] << endl;
//            cout << "widest path: " ;
//            graph->showWidestPath(t);
                query_time += GetCurrentTimeMicroSec();
                cout << "query_time:" << query_time*1e-3 << "ms" << endl;
            }
            else{
                cout <<"Wrong fomat!"<<endl;
                cin.clear();
                cin.ignore();
            }
        }
    }
    void NormalBenchmark(int argc, char **argv){
        if (argc != 2) {
            cerr << "usage: normalWidestPathQuery GRAPH" << endl;
            exit(EXIT_FAILURE);
        }
        int kNumQueries = 100;
        Graph *graph = new Graph();
        graph->constructGraph(argv[1]);
        cout << "graph loaded!" << endl;
        cout << "vertex num: " << graph->V  << ", edge num: " <<  graph->E  << endl;

        cout << "Querying randomly 100 times..." << endl;
        uint32_t V = graph->V;

        vector<int> vs(kNumQueries), ws(kNumQueries);
        for (int i = 0; i < kNumQueries; ++i) {
            vs[i] = rand() % V;
            ws[i] = rand() % V;
        }

        double time_start = GetCurrentTimeSec();
        for (int i = 0; i < kNumQueries; ++i) {
            graph->widsetPath(vs[i], ws[i]);
        }
        double elapsed_time = GetCurrentTimeSec() - time_start;
        cout << "average query time: "
             << elapsed_time / kNumQueries * 1E6
             << " microseconds" << endl;

        exit(EXIT_SUCCESS);
    }
};

class ReachabilityQuery : public SystemFeatures{
private:
    double GetCurrentTimeMicroSec() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1e6 + tv.tv_usec;
    }
    double GetCurrentTimeSec() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }

    bool LoadFile(const char *filename,vector<pair<uint32_t, uint32_t> > &es){
        ifstream ifs(filename);
        for (uint32_t v, w; ifs >> v >> w; ) {
            es.push_back(make_pair(v, w));
        }
        if (ifs.bad()) return false;
        return true;
    }

//start:v   end:w
    uint32_t BFS(const vector<pair<uint32_t, uint32_t> > &es,uint32_t start,uint32_t end){
        if(start == end) return 1;
        uint32_t V = 0;
        for (size_t i = 0; i < es.size(); ++i) {
            V = max(V, max(es[i].first, es[i].second) + 1);
        }
        if(start < 0 || start >= V || end < 0 || end >= V)  return 0;
        vector<vector<uint32_t> > adj(V);
        //输入是有向图
        for (size_t i = 0; i < es.size(); ++i) {
            uint32_t v = es[i].first, w = es[i].second;
            adj[v].push_back(w);
        }
        queue<uint32_t> que;
        //先设置为全部不可达INT32_MAX
        vector<uint32_t> distance(V,0);
        que.push(start);
        //每个顶点到自身是可达的
        distance[start] = 1;
        while(!que.empty()){
            uint32_t v = que.front();
            que.pop();
            for (size_t i = 0; i < adj[v].size(); ++i) {
                uint32_t w = adj[v][i];
                // 如果等于0，说明顶点w没有被访问过
                if (distance[w] == 0) {
                    que.push(w);
                    distance[w] = 1;
                }
                if(w == end) {return distance[w];}
            }
        }
        return 0;

    }
public:
    void ConstructIndex(int argc, char **argv) override {
        if (argc != 4) {
            cerr << "usage: construct_index graph scc label" << endl;
            exit(EXIT_FAILURE);
        }
        Reachability reachability;
        if(!reachability.ContructIndex(argv[1])){
            cerr << "error: Store failed" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Construct Success!" << endl;
        if(!reachability.StoreIndex(argv[2], argv[3])){
            cerr << "error: Store failed" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Store Index Success!" << endl;
    }

    void QueryIndex(int argc, char **argv) override {
        if (argc != 4) {
            cerr << "usage: command graph scc label" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load file, waiting ... " << endl;
        Reachability reachability;
        if(!reachability.LoadGraph(argv[1]) || !reachability.LoadIndex(argv[2], argv[3])){
            cerr << "error: Load failed" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load sucess ! " << endl;
        int inNodeNum, outNodeNum;
        while(true){
            if(cin >> outNodeNum >> inNodeNum){
                double query_time = 0;
                query_time = -GetCurrentTimeMicroSec();
                cout << reachability.query2(outNodeNum, inNodeNum) << endl; //输出查询结果
                query_time += GetCurrentTimeMicroSec();
                cout << "query_time:" << query_time << "us" << endl;
            }
            else{
                cout <<"Wrong fomat!"<<endl;
                cin.clear();
                cin.ignore();
            }
        }
    }

    void PllBenchmark(int argc, char **argv) override {
        if (argc != 4) {
            cerr << "usage: command graph scc label" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load file, waiting ... " << endl;
        Reachability reachability;
        if(!reachability.LoadGraph(argv[1]) || !reachability.LoadIndex(argv[2], argv[3])){
            cerr << "error: Load failed" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load sucess ! " << endl;
        cout << "Querying randomly 1000000 times..." << endl;

        int kNumQueries = 1000000;
        vector<int> vs(kNumQueries), ws(kNumQueries);
        for (int i = 0; i < kNumQueries; ++i) {
            vs[i] = rand() % reachability.getVerticesNum();
            ws[i] = rand() % reachability.getVerticesNum();
        }

        clock_t start = clock();
        for (int i = 0; i < kNumQueries; i++){
            reachability.query2(vs[i], ws[i]);
        }
        clock_t finish = clock();
        cout << "Query time(1000000 times): " << (finish - start) / (double)CLOCKS_PER_SEC << " seconds" << endl; //输出100000次查询的时间
        cout << "Average time: " << (finish - start) / (double)CLOCKS_PER_SEC << " microseconds" << endl;
    }

    void NormalQuery(int argc, char **argv) override {
        if (argc != 2) {
            cerr << "usage: normalReachabilityQuery GRAPH" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load File..." << endl;
        vector<pair<uint32_t, uint32_t> > es;
        if(!LoadFile(argv[1],es)){cerr << "Loading failed!" << endl;return;}
        cout << "Load success!" << endl;
        int u,v;
        while(true){
            if(cin >> u >> v){
                double query_time = -GetCurrentTimeMicroSec();
                cout << "reachability: " << BFS(es, u, v) << endl;
                query_time += GetCurrentTimeMicroSec();
                cout << "query_time:" << query_time * 1e-3 << "ms" << endl;
            }
            else{
                cout <<"Wrong fomat!"<<endl;
                cin.clear();
                cin.ignore();
            }
        }
    }

    void NormalBenchmark(int argc, char **argv) override {
        if (argc != 2) {
            cerr << "usage: normalReachabilityQuery GRAPH" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Load File..." << endl;
        vector<pair<uint32_t, uint32_t> > es;
        if(!LoadFile(argv[1],es)){cerr << "Loading failed!" << endl;return;}
        cout << "Load success!" << endl;

        int kNumQueries = 100;
        cout << "Querying randomly 100 times..." << endl;
        uint32_t V = 0;
        for (size_t i = 0; i < es.size(); ++i) {
            V = max(V, max(es[i].first, es[i].second) + 1);
        }
        vector<int> vs(kNumQueries), ws(kNumQueries);
        for (int i = 0; i < kNumQueries; ++i) {
            vs[i] = rand() % V;
            ws[i] = rand() % V;
        }
        double time_start = GetCurrentTimeSec();
        for (int i = 0; i < kNumQueries; ++i) {
            BFS(es, vs[i], ws[i]);
        }
        double elapsed_time = GetCurrentTimeSec() - time_start;
        cout << "average query time: "
             << elapsed_time / kNumQueries * 1E6
             << " microseconds" << endl;

        exit(EXIT_SUCCESS);
    }
};

#endif
