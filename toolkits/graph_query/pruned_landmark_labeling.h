#ifndef PRUNED_LANDMARK_LABELING_H_
#define PRUNED_LANDMARK_LABELING_H_

#include <malloc.h>
#include <sys/time.h>
#include <climits>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <algorithm>
#include <fstream>
#include <utility>

//
// NOTE: 仅支持无向无权图的输入，进行最短路径距离查询
//

class PrunedLandmarkLabeling{
public:
    // 从原始图构建索引，给定的数据应该是边的集合.
    // 结点id应该是从0开始的整数,构建成功返回true。
    bool ConstructIndex(const std::vector<std::pair<uint32_t, uint32_t> > &es);
    bool ConstructIndex(std::istream &ifs);
    bool ConstructIndex(const char *filename);

    // 如果结点v和w可达，返回之间的距离。否则返回INT_MAX
    inline uint32_t QueryDistance(uint32_t v, uint32_t w);

    // 加载索引 ，成功返回|true| 
    bool LoadIndex(std::istream &ifs);
    bool LoadIndex(const char *filename);

    // Stores the index. Returns |true| when successful.
    bool StoreIndex(std::ostream &ofs);
    bool StoreIndex(const char *filename);

    uint32_t GetNumVertices() { return num_v_; }
    void Free();
    void PrintStatistics();

    PrunedLandmarkLabeling()
            : num_v_(0), index_(NULL), time_load_(0), time_indexing_(0) {}
    virtual ~PrunedLandmarkLabeling() {
        Free();
    }

private:
    static const uint32_t INT32;  // For unreachable pairs

    struct index_t {
        uint32_t *spt_v;
        uint32_t *spt_d;
    };

    uint32_t num_v_;
    index_t *index_;

    double GetCurrentTimeSec() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }

    // Statistics
    double time_load_, time_indexing_;
};

const uint32_t PrunedLandmarkLabeling::INT32 = 100;

bool PrunedLandmarkLabeling
::ConstructIndex(const char *filename) {
    std::ifstream ifs(filename);
    return ifs && ConstructIndex(ifs);
}

bool PrunedLandmarkLabeling
::ConstructIndex(std::istream &ifs) {
    std::vector<std::pair<uint32_t, uint32_t> > es;
    for (uint32_t v, w; ifs >> v >> w; ) {
        es.push_back(std::make_pair(v, w));
    }
    if (ifs.bad()) return false;
    ConstructIndex(es);
    return true;
}

bool PrunedLandmarkLabeling
::ConstructIndex(const std::vector<std::pair<uint32_t, uint32_t> > &es) {
    //
    // Prepare the adjacency list and index space
    //
    Free();
    time_load_ = -GetCurrentTimeSec();
    uint32_t &V = num_v_;
    V = 0;
    for (size_t i = 0; i < es.size(); ++i) {
        V = std::max(V, std::max(es[i].first, es[i].second) + 1);
    }
    std::vector<std::vector<uint32_t> > adj(V);
    for (size_t i = 0; i < es.size(); ++i) {
        uint32_t v = es[i].first, w = es[i].second;
        adj[v].push_back(w);
        adj[w].push_back(v);
    }
    time_load_ += GetCurrentTimeSec();

    index_ = (index_t*)malloc(V * sizeof(index_t));
    if (index_ == NULL) {
        num_v_ = 0;
        return false;
    }
    for (uint32_t v = 0; v < V; ++v) {
        index_[v].spt_v = NULL;
        index_[v].spt_d = NULL;
    }

    //
    // Order vertices by decreasing order of degree
    //
    time_indexing_ = -GetCurrentTimeSec();
    std::vector<uint32_t> inv(V);  // new label -> old label
    {
        // Order
        std::vector<std::pair<float, uint32_t> > deg(V);
        for (uint32_t v = 0; v < V; ++v) {
            // We add a random value here to diffuse nearby vertices
            deg[v] = std::make_pair(adj[v].size() + float(rand()) / RAND_MAX, v);
        }
        std::sort(deg.rbegin(), deg.rend());
        for (uint32_t i = 0; i < V; ++i) inv[i] = deg[i].second;

        // Relabel the vertex IDs
        std::vector<uint32_t> rank(V);
        for (uint32_t i = 0; i < V; ++i) rank[deg[i].second] = i;
        std::vector<std::vector<uint32_t> > new_adj(V);
        for (uint32_t v = 0; v < V; ++v) {
            for (size_t i = 0; i < adj[v].size(); ++i) {
                new_adj[rank[v]].push_back(rank[adj[v][i]]);
            }
        }
        adj.swap(new_adj);
    }


    //
    // Pruned labeling
    //
    std::vector<bool> usd(V, false);  // Used as root? (in new label)
    {
        // Sentinel (V, INT32) is added to all the vertices
        std::vector<std::pair<std::vector<uint32_t>, std::vector<uint32_t> > >
                tmp_idx(V, make_pair(std::vector<uint32_t>(1, V),
                                     std::vector<uint32_t>(1, INT32)));

        std::vector<bool> vis(V);
        std::vector<uint32_t> que(V);
        std::vector<uint32_t> dst_r(V + 1, INT32);

        for (uint32_t r = 0; r < V; ++r) {
            if (usd[r]) continue;
            const std::pair<std::vector<uint32_t>, std::vector<uint32_t> >
                    &tmp_idx_r = tmp_idx[r];
            for (size_t i = 0; i < tmp_idx_r.first.size(); ++i) {
                dst_r[tmp_idx_r.first[i]] = tmp_idx_r.second[i];
            }

            uint32_t que_t0 = 0, que_t1 = 0, que_h = 0;
            que[que_h++] = r;
            vis[r] = true;
            que_t1 = que_h;

            for (uint32_t d = 0; que_t0 < que_h; ++d) {
                for (uint32_t que_i = que_t0; que_i < que_t1; ++que_i) {
                    uint32_t v = que[que_i];
                    std::pair<std::vector<uint32_t>, std::vector<uint32_t> >
                            &tmp_idx_v = tmp_idx[v];

                    // Prune?
                    if (usd[v]) continue;
                    for (size_t i = 0; i < tmp_idx_v.first.size(); ++i) {
                        uint32_t w = tmp_idx_v.first[i];
                        uint32_t td = tmp_idx_v.second[i] + dst_r[w];
                        if (td <= d) goto pruned;
                    }

                    // Traverse
                    tmp_idx_v.first .back() = r;
                    tmp_idx_v.second.back() = d;
                    tmp_idx_v.first .push_back(V);
                    tmp_idx_v.second.push_back(INT32);
                    for (size_t i = 0; i < adj[v].size(); ++i) {
                        uint32_t w = adj[v][i];
                        if (!vis[w]) {
                            que[que_h++] = w;
                            vis[w] = true;
                        }
                    }
                    pruned:
                    {}
                }

                que_t0 = que_t1;
                que_t1 = que_h;
            }

            for (uint32_t i = 0; i < que_h; ++i) vis[que[i]] = false;
            for (size_t i = 0; i < tmp_idx_r.first.size(); ++i) {
                dst_r[tmp_idx_r.first[i]] = INT32;
            }
            usd[r] = true;
        }

        for (uint32_t v = 0; v < V; ++v) {
            uint32_t k = tmp_idx[v].first.size();
            index_[inv[v]].spt_v = (uint32_t*)malloc(k * sizeof(uint32_t));
            index_[inv[v]].spt_d = (uint32_t *)malloc(k * sizeof(uint32_t ));
            if (!index_[inv[v]].spt_v || !index_[inv[v]].spt_d) {
                Free();
                return false;
            }
            for (uint32_t i = 0; i < k; ++i) {
                index_[inv[v]].spt_v[i] = tmp_idx[v].first[i];
                index_[inv[v]].spt_d[i] = tmp_idx[v].second[i];
            }
            std::vector<uint32_t>().swap(tmp_idx[v].first);
            std::vector<uint32_t>().swap(tmp_idx[v].second);
            //tmp_idx[v].first.clear();
            //tmp_idx[v].second.clear();
        }
    }

    time_indexing_ += GetCurrentTimeSec();
    return true;
}

uint32_t PrunedLandmarkLabeling
::QueryDistance(uint32_t v, uint32_t w) {
    if (v >= num_v_ || w >= num_v_) return v == w ? 0 : INT_MAX;

    const index_t &idx_v = index_[v];
    const index_t &idx_w = index_[w];
    uint32_t d = INT32;

    for (uint32_t i1 = 0, i2 = 0; ; ) {
        uint32_t v1 = idx_v.spt_v[i1], v2 = idx_w.spt_v[i2];
        if (v1 == v2) {
            if (v1 == num_v_) break;  // Sentinel
            uint32_t td = idx_v.spt_d[i1] + idx_w.spt_d[i2];
            if (td < d) d = td;
            ++i1;
            ++i2;
        } else {
            i1 += v1 < v2 ? 1 : 0;
            i2 += v1 > v2 ? 1 : 0;
        }
    }

    if (d > INT32 - 2) d = INT_MAX;//why -2?
    return d;
}

bool PrunedLandmarkLabeling
::LoadIndex(const char *filename) {
    std::ifstream ifs(filename);
    return ifs && LoadIndex(ifs);
}

bool PrunedLandmarkLabeling
::LoadIndex(std::istream &ifs) {
    Free();

    uint32_t num_v;
    ifs.read((char*)&num_v,   sizeof(num_v));
    num_v_ = num_v;
    if (ifs.bad()) {
        num_v_ = 0;
        return false;
    }

    index_ = (index_t*)malloc(num_v * sizeof(index_t));
    if (index_ == NULL) {
        num_v_ = 0;
        return false;
    }
    for (uint32_t v = 0; v < num_v_; ++v) {
        index_[v].spt_v = NULL;
        index_[v].spt_d = NULL;
    }

    for (uint32_t v = 0; v < num_v_; ++v) {
        index_t &idx = index_[v];

        uint32_t s;
        ifs.read((char*)&s, sizeof(s));
        if (ifs.bad()) {
            Free();
            return false;
        }

        idx.spt_v = (uint32_t*)malloc(s * sizeof(uint32_t));
        idx.spt_d = (uint32_t *)malloc(s * sizeof(uint32_t ));
        if (!idx.spt_v || !idx.spt_d) {
            Free();
            return false;
        }

        for (uint32_t i = 0; i < s; ++i) {
            ifs.read((char*)&idx.spt_v[i], sizeof(idx.spt_v[i]));
            ifs.read((char*)&idx.spt_d[i], sizeof(idx.spt_d[i]));
        }
    }

    return ifs.good();
}

bool PrunedLandmarkLabeling
::StoreIndex(const char *filename) {
    std::ofstream ofs(filename);
    return ofs && StoreIndex(ofs);
}

bool PrunedLandmarkLabeling
::StoreIndex(std::ostream &ofs) {
    uint32_t num_v = num_v_;
    ofs.write((const char*)&num_v,   sizeof(num_v));

    for (uint32_t v = 0; v < num_v_; ++v) {
        index_t &idx = index_[v];


        uint32_t s;
        for (s = 1; idx.spt_v[s - 1] != num_v; ++s) continue;  // Find the sentinel
        ofs.write((const char*)&s, sizeof(s));
        for (uint32_t i = 0; i < s; ++i) {
            uint32_t l = idx.spt_v[i];
            uint32_t  d = idx.spt_d[i];
            ofs.write((const char*)&l, sizeof(l));
            ofs.write((const char*)&d, sizeof(d));
        }
    }

    return ofs.good();
}

void PrunedLandmarkLabeling
::Free() {
    for (uint32_t v = 0; v < num_v_; ++v) {
        free(index_[v].spt_v);
        free(index_[v].spt_d);
    }
    free(index_);
    index_ = NULL;
    num_v_ = 0;
}

void PrunedLandmarkLabeling
::PrintStatistics() {
    std::cout << "load time: "     << time_load_     << " seconds" << std::endl;
    std::cout << "indexing time: " << time_indexing_ << " seconds" << std::endl;

    double s = 0.0;
    for (uint32_t v = 0; v < num_v_; ++v) {
        for (uint32_t i = 0; index_[v].spt_v[i] != uint32_t(num_v_); ++i) {
            ++s;
        }
    }
    s /= num_v_;
    std::cout << "average normal label size: " << s << std::endl;
}
#endif //PRUNED_LANDMARK_LABELING_H
