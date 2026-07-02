// yoo2i's work
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

typedef struct {
    int valid;
    long long tag;
    int lru_counter;
} CacheLine;

int main() {
    int opt;
    int s = 0, E = 0, b = 0;
    int verbose = 0;
    char *trace_file = NULL;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 's':
                s = atoi(optarg); // 自动拿到 s 后面的值
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg; // 自动拿到 trace 文件名字符串
                break;
            case 'v':
                verbose = 1; // 开启详细模式
                break;
            case 'h':
                printf("Usage: ./csim [-hv] -s <s> -E <E> -b <b> -t <trace>\n");
                printf("-h: Optional help flag that prints usage info\n");
                printf("-v: Optional verbose flag that displays trace info\n");
                printf("-s <s>: Number of set index bits (S = 2^s is the number of sets)\n");
                printf("-E <E>: Associativity (number of lines per set)\n");
                printf("-b <b>: Number of block bits (B = 2^b is the block size)\n");
                printf("-t <tracefile>: Name of the valgrind trace to replay\n");
                break;
            default:
                printf("Fuck you,you son of bitch!\n")// 处理非法输入
                exit(1);
        }
    }

    //create cache
    int setNums = 2 ^ s;
    CacheLine** Cache = (CacheLine **)malloc(sizeof(CacheLine *) * setNums);
    for(int i = 0; i < setNums; i++) {
        Cache[i] = (CacheLine *)malloc(sizeof(CacheLine) * E);
    }
    printSummary(0, 0, 0);
    return 0;
}
