#include <iostream>
#include <cstring>
#include "SystemFeatures.h"

using namespace std;

/**
 * @param argc
 * @param argv 至少有两个输入参数，第一个参数argv[1]是整数，表示算法选择；
 * @return
 * 每个算法支持5中功能选择：normalquery normalbenchmark constructindex queryindex pllbenchmark
 */
int main(int argc, char **argv) {
    if(argc <= 2){
        //如果输入参数小于等于2
        cerr << "usage error!" << endl;
    }
    SystemFeatures *s = NULL;
    if(strcmp(argv[1], "1") == 0){
        //说明选择的是无向无权图的最短路径距离索引算法
        s = new ShortestDistance();
    }else if(strcmp(argv[1], "2") == 0){
        //说明选择的是有向有权图的最宽路径距离索引算法
        s = new WidestDistance();
    }
    else if(strcmp(argv[1], "3") == 0){
        //说明选择的是有向无权图的可达性索引算法
        s = new ReachabilityQuery();
    }else{
        cerr << "usage error!" << endl;
        return 0;
    }

    if(strcmp(argv[2], "constructindex") == 0){
        if(strcmp(argv[1], "3") == 0 ){
            //可达性的constructindex参数更多,单独处理
            if(argc == 6){
                char *parameter[4] = {argv[0], argv[3], argv[4], argv[5]};
                s->ConstructIndex(argc - 2, parameter);
                return 0;
            }else{
                cerr << "usage error!" << endl;
                exit(EXIT_FAILURE);
            }
        }
        if(argc != 5){
            cerr << "usage error!" << endl;
            exit(EXIT_FAILURE);
        }
        char *parameter[3] = {argv[0], argv[3], argv[4]};
        s->ConstructIndex(argc - 2, parameter);
    }else if(strcmp(argv[2], "queryindex") == 0) {
        if(strcmp(argv[1], "3") == 0 ){
            //可达性的queryindex参数更多,单独处理
            if(argc == 6){
                char *parameter[4] = {argv[0], argv[3], argv[4], argv[5]};
                s->QueryIndex(argc - 2, parameter);
                return 0;
            }else{
                cerr << "usage error!" << endl;
                exit(EXIT_FAILURE);
            }
        }
        if (argc != 4) {
            cerr << "usage error!" << endl;
            exit(EXIT_FAILURE);
        }
        char *parameter[2] = {argv[0], argv[3]};
        s->QueryIndex(argc - 2, parameter);
    }else if(strcmp(argv[2], "pllbenchmark") == 0) {
        if(strcmp(argv[1], "3") == 0 ){
            //可达性的queryindex参数更多,单独处理
            if(argc == 6){
                char *parameter[4] = {argv[0], argv[3], argv[4], argv[5]};
                s->PllBenchmark(argc - 2, parameter);
                return 0;
            }else{
                cerr << "usage error!" << endl;
                exit(EXIT_FAILURE);
            }
        }
        if (argc != 4) {
            cerr << "usage error!" << endl;
            exit(EXIT_FAILURE);
        }
        char *parameter[2] = {argv[0], argv[3]};
        s->PllBenchmark(argc - 2, parameter);
    }else if(strcmp(argv[2], "normalquery") == 0) {
        if (argc != 4) {
            cerr << "usage error!" << endl;
            exit(EXIT_FAILURE);
        }
        char *parameter[2] = {argv[0], argv[3]};
        s->NormalQuery(argc - 2, parameter);
    }else if(strcmp(argv[2], "normalbenchmark") == 0) {
        if (argc != 4) {
            cerr << "usage error!" << endl;
            exit(EXIT_FAILURE);
        }
        char *parameter[2] = {argv[0], argv[3]};
        s->NormalBenchmark(argc - 2, parameter);
    }else {
        cerr << "usage error!" << endl;
        exit(EXIT_FAILURE);
    }
    return 0;
}