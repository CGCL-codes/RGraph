#ifndef PLLWIDESTPATH_H
#define PLLWIDESTPATH_H

#include <iostream>
#include <set>
#include <vector>
#include <fstream>
#include <map>
#include <sys/time.h>
#include <algorithm>
using namespace std;

typedef pair<float, int> iPair; // iPair的float保存的是边的权值

class PllWidestPath{
private://变量后面带_表示是私有变量
    static const float INF;     //-INF表示结点不可达，INF表示最宽路径最宽的值，如 2->2 的最宽路径距离就是INF

    struct index_out{//顶点的out索引结构体
        uint32_t *spt_v;
        float *spt_d;
    };

    struct index_in{//顶点的in索引结构体
        uint32_t *spt_v;
        float *spt_d;
    };

    uint32_t num_v_; //顶点数量
    index_out *index_out_;    //保存的out索引
    index_in  *index_in_;    //保存的in索引
    double time_load_,time_indexing_; //统计数据加载时间和索引构建时间

public:
    // 顶点ID 从0开始，构建索引成功返回 |true|
    bool constructIndex(const char *filename);
    bool constructIndex(istream &ifs);
    bool constructIndex(const vector<tuple<uint32_t ,uint32_t ,float > > &es);

    //返回顶点|v|到|w|的最宽距离，如果顶点没有有路径，返回|-INF
    inline float queryDistance(uint32_t u, uint32_t w);

    //加载索引，当成功时返回|true|
    bool loadIndex(const char *filename);
    bool loadIndex(istream &ifs);

    //存储索引到磁盘文件，成功返回|true|
    bool storeIndex(const char *filename);
    bool storeIndex(ostream &ofs);

    uint32_t getVerticeNum(){  //返回顶点数量
        return num_v_;
    }

    void printStatistic();  //打印构建索引时间s，查询平均时间us
    void freeMemory();  //释放结构体内存，并将默认初始类变量清零

    double GetCurrentTimeSec() {    //获取当前系统时间，单位秒（s）
        struct timeval tv{};
        gettimeofday(&tv, nullptr);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }

    PllWidestPath():num_v_(0),index_in_(nullptr), index_out_(nullptr), time_load_(0), time_indexing_(0){};
    virtual ~PllWidestPath(){
        freeMemory();
    }
};

const float  PllWidestPath::INF = 99999;

bool PllWidestPath
::constructIndex(const char *filename) {
    ifstream ifs(filename);
    return ifs && constructIndex(ifs);
}

bool PllWidestPath
::constructIndex(istream &ifs){
    vector<tuple<uint32_t ,uint32_t ,float > > es; //边的集合
    uint32_t u,v;
    float d;
    for(;ifs >> u >> v >> d;){
        es.push_back(make_tuple(u ,v ,d));
    }
    if (ifs.bad()) return false;
    constructIndex(es);
    return true;
}

bool PllWidestPath
::constructIndex(const vector<tuple<uint32_t ,uint32_t ,float > > &es){
    freeMemory();
    time_load_ = -GetCurrentTimeSec();
    uint32_t &V = num_v_;
    V = 0;
    //获取顶点最大的ID，顶点从0开始，V = ID + 1
    for(uint32_t i = 0; i < es.size(); i++){
        V = max(V, max(get<0>(es[i]), get<1>(es[i])) + 1);
    }
    cout << "graph.V = " << V << endl;
    cout << "graph.E = " << es.size() << endl;
    //将边存储为邻接表
    vector<vector<pair<uint32_t , float > > > adj_out(V);
    vector<vector<pair<uint32_t , float > > > adj_in(V);
    for(uint32_t i = 0; i < es.size(); i++){
        uint32_t v = get<0>(es[i]), w = get<1>(es[i]);
        float d = get<2>(es[i]);
        adj_out[v].push_back(make_pair(w, d));
        adj_in[w].push_back(make_pair(v, d));
    }
    time_load_ += GetCurrentTimeSec();

    index_in_ = (index_in*)malloc(V * sizeof(index_in));
    index_out_ = (index_out*)malloc(V * sizeof(index_out));
    if(index_out_ == nullptr || index_in_ == nullptr){
        num_v_ = 0;
        return false;
    }
    //初始化索引的空间
    for(uint32_t v = 0; v < V; v++){
        index_out_[v].spt_v = nullptr;
        index_out_[v].spt_d = nullptr;
        index_in_[v].spt_v = nullptr;
        index_in_[v].spt_d = nullptr;
    }

    time_indexing_ = -GetCurrentTimeSec();
    // 有向图按照 入度*出度 进行降序排序
    //todo
    vector<uint32_t> newToOld(V + 1);  // new label -> old label
    for(uint32_t i = 0; i <= V; i++ ){
        newToOld[i] = i;
    }
    {
        // 统计每个顶点 入度 * 出度
        vector<pair<double , uint32_t > > deg(V);
        for(uint32_t i = 0; i < V; i++){
            deg[i] = make_pair((adj_in[i].size() + 1) * (adj_out[i].size() + 1) + double(rand()) / RAND_MAX, i);
        }
        sort(deg.rbegin(), deg.rend());     // 降序排序
        for(uint32_t i = 0; i < V; i++) newToOld[i] = deg[i].second;

        // 重新对顶点id进行排序
        vector<uint32_t> rank(V);
        for(uint32_t i = 0; i < V; i++) rank[deg[i].second] = i;
        vector<vector<pair<uint32_t , float> > > new_adj_out(V);
        vector<vector<pair<uint32_t , float> > > new_adj_in(V);

        for(uint32_t v = 0; v < V; v++){
            for(size_t i = 0; i < adj_out[v].size(); ++i){
                new_adj_out[rank[v]].push_back(make_pair(rank[adj_out[v][i].first], adj_out[v][i].second));
            }
        }
        adj_out.swap(new_adj_out);

        for(uint32_t v = 0; v < V; v++){
            for(size_t i = 0; i < adj_in[v].size(); ++i){
                new_adj_in[rank[v]].push_back(make_pair(rank[adj_in[v][i].first], adj_in[v][i].second));
            }
        }
        adj_in.swap(new_adj_in);
    }
    cout << "sorted over" << endl;


    //构建索引,剪枝核心算法
    vector<bool> usd(V, false); //usd用来标记顶点是否已经被当做root产生索引了 (in new label)
    {
        //临时索引，需要添加一个Sentinel (V, -INF),构建入索引和出度索引,0表示没有最宽路径
        vector<pair<vector<uint32_t >, vector<float > > >
                tmp_index_in(V, make_pair(vector<uint32_t >(1, V),
                                          vector<float >(1, -INF)));
        vector<pair<vector<uint32_t >, vector<float > > >
                tmp_index_out(V, make_pair(vector<uint32_t >(1, V),
                                           vector<float >(1, -INF)));
        vector<float > arr_out_r(V + 1, -INF); //顶点r到其他顶点的最宽距离，多了一个Sentinel，故为V+1
        vector<float > arr_in_r(V + 1, -INF);  //其他顶点到r的最宽距离
        set< iPair, greater<iPair> > maxHeap1;  //注意将权值放在pair的第一个位置，构建最大堆
        set< iPair, greater<iPair> > maxHeap2;

        vector<float > dst_out_r(V + 1, 0);  //记录上一轮迭代的r的索引
        vector<float > dst_in_r(V + 1, 0);
        for(uint32_t r = 0; r < V; r++){
            if(r == V / 10) cout << "10%" << endl;
            else if(r == V * 2/ 10) cout << "20%" << endl;
            else if(r == V * 3 / 10) cout << "30%" << endl;
            else if(r == V * 4 / 10) cout << "40%" << endl;
            else if(r == V / 2) cout << "50%" << endl;
            else if(r == V * 7 / 10) cout << "70%" << endl;
            else if(r == V * 9 / 10) cout << "90%" << endl;
            else if(r == V - 1) cout << "100%" << endl;
            if(usd[r]) continue;
            //初始化最宽距离数组
            for(uint32_t i = 0; i < V + 1; i++){
                arr_out_r[i] = -INF;
                arr_in_r[i]  = -INF;
            }
            arr_in_r[r] = INF;
            arr_out_r[r] = INF;

            //上一轮所留下来的r索引,最宽距离保存在dst_out_t和dst_in_t中
            const pair<vector<uint32_t >, vector<float> >
                    &tmp_index_out_r = tmp_index_out[r];
            const pair<vector<uint32_t >, vector<float> >
                    &tmp_index_in_r = tmp_index_in[r];
            for(uint32_t i = 0; i < tmp_index_out_r.first.size(); i++){
                dst_out_r[tmp_index_out_r.first[i]] = tmp_index_out_r.second[i];
            }
            for(uint32_t i = 0; i < tmp_index_in_r.first.size(); i++){
                dst_in_r[tmp_index_in_r.first[i]] = tmp_index_in_r.second[i];
            }

            //正向执行修改的Dijkstra算法
            maxHeap1.insert(make_pair(INF, r));
            while(!maxHeap1.empty()){
                iPair tmp = *(maxHeap1.begin());
                maxHeap1.erase(maxHeap1.begin());
                int max = tmp.second;
                pair<vector<uint32_t >, vector<float> >
                        &tmp_index_in_max = tmp_index_in[max];

                //判断是否剪枝
                if(usd[max]) goto pruned1;
                for(uint32_t i = 0; i < tmp_index_in_max.first.size(); i++){
                    uint32_t w = tmp_index_in_max.first[i];
                    float   td = dst_out_r[w] < tmp_index_in_max.second[i] ? dst_out_r[w] : tmp_index_in_max.second[i];
                    float    d = arr_out_r[max];
                    if(td >= d) goto pruned1;
                }
                //将该索引保存在tmp_index_in_max中
                tmp_index_in_max.first .back() = r;
                tmp_index_in_max.second.back() = arr_out_r[max];
                tmp_index_in_max.first .push_back(V);
                tmp_index_in_max.second.push_back(-INF);

                for(auto i = adj_out[max].begin(); i != adj_out[max].end(); i++){
                    uint32_t v = (*i).first;
                    float weight = (*i).second;
                    float dis = arr_out_r[max] > weight ? weight : arr_out_r[max];
                    if(arr_out_r[v] < dis){
                        if(arr_out_r[v] != -INF){
                            maxHeap1.erase(maxHeap1.find(make_pair(arr_out_r[v], v)));//说明最大堆的v需要更新其value
                        }
                        arr_out_r[v] = dis;
                        maxHeap1.insert(make_pair(arr_out_r[v], v));
                    }
                }
                pruned1:
                {};
            }
            //逆向执行修改的Dijkstra算法
            maxHeap2.insert(make_pair(INF, r));
            while(!maxHeap2.empty()){
                iPair tmp = *(maxHeap2.begin());
                maxHeap2.erase(maxHeap2.begin());
                int max = tmp.second;
                pair<vector<uint32_t >, vector<float> >
                        &tmp_index_out_max = tmp_index_out[max];

                //判断是否剪枝
                if(usd[max]) goto pruned2;
                for(uint32_t i = 0; i < tmp_index_out_max.first.size(); i++){
                    uint32_t w = tmp_index_out_max.first[i];
                    float   td = dst_in_r[w] < tmp_index_out_max.second[i] ? dst_in_r[w] : tmp_index_out_max.second[i];
                    float    d = arr_in_r[max];
                    if(td >= d) goto pruned2;
                }
                //将该索引保存在tmp_index_out_max中
                tmp_index_out_max.first .back() = r;
                tmp_index_out_max.second.back() = arr_in_r[max];
                tmp_index_out_max.first .push_back(V);
                tmp_index_out_max.second.push_back(-INF);

                for(auto i = adj_in[max].begin(); i != adj_in[max].end(); i++){
                    uint32_t v = (*i).first;
                    float weight = (*i).second;
                    float dis = arr_in_r[max] > weight ? weight : arr_in_r[max];
                    if(arr_in_r[v] < dis){
                        if(arr_in_r[v] != -INF){
                            maxHeap2.erase(maxHeap2.find(make_pair(arr_in_r[v], v)));//说明最大堆的v需要更新其value
                        }
                        arr_in_r[v] = dis;
                        maxHeap2.insert(make_pair(arr_in_r[v], v));
                    }
                }
                pruned2:
                {};
            }

            //将保存索引距离的数组还原成-INF
            for(uint32_t i = 0; i < tmp_index_out_r.first.size(); i++){
                dst_out_r[tmp_index_out_r.first[i]] = -INF;
            }
            for(uint32_t i = 0; i < tmp_index_in_r.first.size(); i++){
                dst_in_r[tmp_index_in_r.first[i]] = -INF;
            }
            usd[r] = true;
        }
        //上述for循环已经将所有索引保存在tmp_index_in、tmp_index_out中了
        for(uint32_t v = 0; v < V; v++){
            uint32_t k = tmp_index_in[v].first.size();
            index_in_[newToOld[v]].spt_v = (uint32_t*)malloc(k * sizeof(uint32_t));
            index_in_[newToOld[v]].spt_d = (float*)malloc(k * sizeof(float));
            if(!index_in_[newToOld[v]].spt_v || !index_in_[newToOld[v]].spt_d){
                freeMemory();
                return false;
            }
            for(uint32_t i = 0; i < k; i++){
                //如果经过排序，为了保证索引的内容是顺序的，spt_v必须是排序后的id，因为是按照排序的顺序进行索引算法的
                index_in_[newToOld[v]].spt_v[i] = tmp_index_in[v].first[i];
                index_in_[newToOld[v]].spt_d[i] = tmp_index_in[v].second[i];
            }
            vector<uint32_t>().swap(tmp_index_in[v].first);
            vector<float>().swap(tmp_index_in[v].second);

            k = tmp_index_out[v].first.size();
            index_out_[newToOld[v]].spt_v = (uint32_t*)malloc(k * sizeof(uint32_t));
            index_out_[newToOld[v]].spt_d = (float*)malloc(k * sizeof(float));
            if(!index_out_[newToOld[v]].spt_v || !index_out_[newToOld[v]].spt_d){
                freeMemory();
                return false;
            }
            for(uint32_t i = 0; i < k; i++){
                index_out_[newToOld[v]].spt_v[i] = tmp_index_out[v].first[i];
                index_out_[newToOld[v]].spt_d[i] = tmp_index_out[v].second[i];
            }
            vector<uint32_t>().swap(tmp_index_out[v].first);
            vector<float>().swap(tmp_index_out[v].second);
        }
    }

    time_indexing_ += GetCurrentTimeSec();
    return true;
}


float PllWidestPath
::queryDistance(uint32_t v, uint32_t w) {
    if (v >= num_v_ || w >= num_v_) return v == w ? INF : 0;
    const index_out &idx_out_v = index_out_[v];
    const index_in &idx_in_w = index_in_[w];
    float d = -INF;
//
//    //打印v的出索引
//    cout << "out index of v: " << endl;
//    for(uint32_t i = 0; ; i++){
//        uint32_t v1 = idx_out_v.spt_v[i];
//        if(v1 == num_v_) break;
//        cout << v << "->" << v1 << ", dis: " << idx_out_v.spt_d[i] << endl;
//    }
//    //打印w的入索引
//    cout << "in index of w: " << endl;
//    for(uint32_t i = 0; ; i++){
//        uint32_t v1 = idx_in_w.spt_v[i];
//        if(v1 == num_v_) break;
//        cout << v1 << "->" << w << ", dis: " << idx_in_w.spt_d[i] << endl;
//    }

    for(uint32_t i1 = 0, i2 = 0; ; ){
        uint32_t v1 = idx_out_v.spt_v[i1], v2 = idx_in_w.spt_v[i2];
        if(v1 == v2){
            if(v1 == num_v_) break;
            float d1 = idx_out_v.spt_d[i1];
            float d2 = idx_in_w.spt_d[i2];
            float td = d1 < d2 ? d1 : d2;
            if(td > d) d = td;
            ++i1;
            ++i2;
        }else{
            i1 += v1 < v2 ? 1 : 0;
            i2 += v1 > v2 ? 1 : 0;
        }
    }
    return d;
}

bool PllWidestPath
::storeIndex(const char *filename) {
    ofstream ofs(filename, ios::out| ios::binary);
    return ofs && storeIndex(ofs);
}

bool PllWidestPath
::storeIndex(ostream &ofs) {
    uint32_t num_v = num_v_;

    ofs.write((const char*)&num_v,   sizeof(num_v));

    for(uint32_t v = 0; v < num_v_; v++){
        index_out &idx_out = index_out_[v];
        uint32_t s;
        for(s = 1; idx_out.spt_v[s - 1] != num_v; s++) continue;
        ofs.write((const char*)&s, sizeof(s));  //s在这里就表示顶点v的出label数量
        for(uint32_t i = 0; i < s; i++){
            uint32_t l = idx_out.spt_v[i];
            float d = idx_out.spt_d[i];
            ofs.write((const char*)&l, sizeof(l));
            ofs.write((const char*)&d, sizeof(d));
        }

        index_in &idx_in = index_in_[v];
        for(s = 1; idx_in.spt_v[s - 1] != num_v; s++) continue;
        ofs.write((const char*)&s, sizeof(s));  //s在这里就表示顶点v的入label数量
        for(uint32_t i = 0; i < s; i++){
            uint32_t l = idx_in.spt_v[i];
            float d = idx_in.spt_d[i];
            ofs.write((const char*)&l, sizeof(l));
            ofs.write((const char*)&d, sizeof(d));
        }
    }

    return ofs.good();
}

bool PllWidestPath
::loadIndex(const char *filename) {
    ifstream ifs(filename, ios::in| ios::binary);
    return ifs && loadIndex(ifs);
}

bool PllWidestPath
::loadIndex(istream &ifs) {
    freeMemory();
    uint32_t num_v;
    ifs.read((char*)&num_v,   sizeof(num_v));
    num_v_ = num_v;

    if(ifs.bad()){
        num_v_ = 0;
        return false;
    }

    index_out_ = (index_out*)malloc(num_v * sizeof(index_out));
    index_in_ = (index_in*)malloc(num_v * sizeof(index_in));
    if(!index_out_ || !index_in_){
        num_v_ = 0;
        return false;
    }
    for (uint32_t v = 0; v < num_v_; ++v) {
        index_out_[v].spt_v = nullptr;
        index_out_[v].spt_d = nullptr;
        index_in_[v] .spt_v = nullptr;
        index_in_[v] .spt_d = nullptr;
    }

    for (uint32_t v = 0; v < num_v_; ++v){
        index_out &idx_out = index_out_[v];
        index_in  &idx_in  = index_in_[v];
        uint32_t s;
        ifs.read((char*)&s, sizeof(s));
        if (ifs.bad()) {
            freeMemory();
            return false;
        }

        idx_out.spt_v = (uint32_t*)malloc(s * sizeof(uint32_t));
        idx_out.spt_d = (float *)malloc(s * sizeof(float ));
        //idx_out，那么返回false，保证输入文件不符合格式的情况
        if (!idx_out.spt_v || !idx_out.spt_d) {
            freeMemory();
            return false;
        }
        for (uint32_t i = 0; i < s; ++i) {
            ifs.read((char*)&idx_out.spt_v[i], sizeof(idx_out.spt_v[i]));
            ifs.read((char*)&idx_out.spt_d[i], sizeof(idx_out.spt_d[i]));
        }

        ifs.read((char*)&s, sizeof(s));
        if (ifs.bad()) {
            freeMemory();
            return false;
        }

        idx_in.spt_v = (uint32_t*)malloc(s * sizeof(uint32_t));
        idx_in.spt_d = (float *)malloc(s * sizeof(float ));
        //idx_out，那么返回false，保证输入文件不符合格式的情况
        if (!idx_in.spt_v || !idx_in.spt_d) {
            freeMemory();
            return false;
        }
        for (uint32_t i = 0; i < s; ++i) {
            ifs.read((char*)&idx_in.spt_v[i], sizeof(idx_in.spt_v[i]));
            ifs.read((char*)&idx_in.spt_d[i], sizeof(idx_in.spt_d[i]));
        }

    }
    return ifs.good();

}

void PllWidestPath
::freeMemory(){
    for(uint32_t v = 0; v < num_v_; v++){
        free(index_in_[v].spt_v);
        free(index_in_[v].spt_d);
        free(index_out_[v].spt_v);
        free(index_out_[v].spt_d);
    }
    free(index_in_);
    free(index_out_);
    num_v_ = 0;
    index_out_ = nullptr;
    index_in_ = nullptr;
}

void PllWidestPath
::printStatistic() {
    cout << "load time: "     << time_load_     << " seconds" << endl;
    cout << "indexing time: " << time_indexing_ << " seconds" << endl;

    double s = 0.0;
    for (uint32_t v = 0; v < num_v_; ++v) {
        for (uint32_t i = 0; index_out_[v].spt_v[i] != num_v_; ++i) {
            ++s;
        }
        for (uint32_t i = 0; index_in_[v].spt_v[i] != num_v_; ++i) {
            ++s;
        }
    }
    s /= num_v_;
    cout << "average normal label size: " << s << endl;
}


#endif //PLLWIDESTPATH_H
