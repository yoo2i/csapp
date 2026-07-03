// yoo2i's work
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

typedef struct { //c语言不支持在结构体内部直接初始化
    int valid;
    unsigned long long tag;
    int lru_counter; //408 style
} CacheLine;
typedef struct {
    int s;
    int E;
    int b;
    int verbose;
    char *tracefile;

    int S; //setNums
} CacheParams;
int hit = 0;
int miss = 0;
int eviction = 0;

void getOption(int argc, char * const argv[], CacheParams *params) {
    params -> verbose = 0;
    int opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 's':
                params -> s = atoi(optarg);
                params -> S = 1 << params -> s;
                break;
            case 'E':
                params -> E = atoi(optarg);
                break;
            case 'b':
                params -> b = atoi(optarg);
                break;
            case 't':
                params -> tracefile = optarg;
                break;
            case 'v':
                params -> verbose = 1; // 开启详细模式
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
                printf("Fuck you,you son of bitch!\n");
                exit(1);
        }
    }
}

CacheLine** initCache(CacheParams params) {
    CacheLine** Cache = (CacheLine **)malloc(sizeof(CacheLine *) * params.S);
    for(int i = 0; i < params.S; i++) {
        Cache[i] = (CacheLine *)malloc(sizeof(CacheLine) * params.E);

        for (int j = 0; j < params.E; j++) {
            Cache[i][j].valid = 0;
            Cache[i][j].tag = 0;
            Cache[i][j].lru_counter = 0;
        }
    }
    return Cache;
}
void freeCache(CacheLine** cache, CacheParams params) {
    for (int i = 0; i < params.S; i++) {
        free(cache[i]);
    }
    free(cache);
}

//for test
void printCache(CacheLine **cache, CacheParams params) {
    for (int i = 0; i < params.S; i++) {
        for (int j = 0; j < params.E; j++) {
            printf("S: %d E: %d valid: %d tag: %llx lru: %d\n", i, j, cache[i][j].valid, cache[i][j].tag, cache[i][j].lru_counter);
        }
    }
}
void accessCache(CacheLine **cache, CacheParams params, unsigned long long address) {
    unsigned long long setIndex = (address >> params.b) & ((1ULL << params.s) - 1);
    unsigned long long tag = address >> (params.s + params.b);

    int hitFlag = 0;
    int evictFlag = 1;
    for (int i = 0; i < params.E; i++) {
        if (cache[setIndex][i].valid == 1 && cache[setIndex][i].tag == tag) {
            hitFlag = 1;
            hit += 1;

            // lru
            int now = cache[setIndex][i].lru_counter;
            for (int j = 0; j < params.E; j++) {
                if (j == i) {
                    cache[setIndex][j].lru_counter = 0;
                } else if (cache[setIndex][j].lru_counter < now) {
                    cache[setIndex][j].lru_counter += 1;
                }
            }
            break;
        }
    }
    if (hitFlag == 0) {
        miss += 1;
        if (params.verbose == 1) {
            printf("miss ");
        }

        for (int i = 0; i < params.E; i++) {
            if (cache[setIndex][i].valid == 0) {
                evictFlag = 0;
                cache[setIndex][i].valid = 1;
                cache[setIndex][i].tag = tag;
                cache[setIndex][i].lru_counter = 0;

                //lru
                for (int j = 0; j < params.E; j++) {
                    if (j != i) {
                        cache[setIndex][j].lru_counter += 1;
                    }
                }
                break;
            }
        }
        if (evictFlag) {
            eviction += 1;
            if (params.verbose) {
                printf("eviction ");
            }

            // lru
            for (int i = 0; i < params.E; i++) {
                if (cache[setIndex][i].lru_counter == params.E - 1) {
                    cache[setIndex][i].tag = tag;
                    cache[setIndex][i].lru_counter = 0;
                } else {
                    cache[setIndex][i].lru_counter += 1;
                }
            }
        }
    } else if (hitFlag == 1 && params.verbose == 1) {
        printf("hit ");
    }
}
// read every line in a loop and call relevant funtion
void readFile(CacheLine **cache, CacheParams params) {
    FILE *file = fopen(params.tracefile, "r");
    if (file == NULL) {
        printf("can not open file: %s\n", params.tracefile);
        exit(1);
    }

    char line[256];
    char type;
    unsigned long long address;
    int size;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] != ' ') {
            continue;
        }

        sscanf(line, " %c %llx,%d", &type, &address, &size);
        if (params.verbose) {
            printf("%c %llx,%d ", type, address, size);
        }

        if (type == 'L' || type == 'S') {
            accessCache(cache, params, address);
        } else if (type == 'M') {
            accessCache(cache, params, address); // M is equal to Load
            accessCache(cache, params, address); // then Store
        }

        if (params.verbose) {
            printf("\n");
        }
        //printCache(cache, params);
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    CacheParams Params;
    getOption(argc, argv, &Params);

    CacheLine** Cache = initCache(Params);
    
    readFile(Cache, Params);

    freeCache(Cache, Params);

    printSummary(hit, miss, eviction);
    return 0;
}
