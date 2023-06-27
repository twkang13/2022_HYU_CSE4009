/* 2021025205 강태욱 */

#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int s, E, b;
char* t;

typedef struct{
    int v;
    unsigned long tag;
    unsigned int lastUsed;
} block;

typedef block* line;
typedef line* cache_t;

cache_t cache; //make cache

int hits, misses, envictions;
unsigned int time;

FILE* file;

/* find the block to be replaced */
int LRU(unsigned long index){
    unsigned int minLastUsed = 4294967295;
    int replaceBlock = 0;

    for (int i = 0; i < E; i++){
        if (cache[index][i].lastUsed < minLastUsed){
            replaceBlock = i;
            minLastUsed = cache[index][i].lastUsed;
        }
    }

    return replaceBlock;
}

/* initialize cache */
void initializeCache(int row){
    cache = (line*)malloc(sizeof(line) * row);

    for (int i = 0; i < row; i++){
        cache[i] = (block*)malloc(sizeof(block) * E);

        for (int j = 0; j < E; j++){
            cache[i][j].v = 0;
            cache[i][j].tag = 0;
            cache[i][j].lastUsed = 0;
        }
    }
}

void accessCache(unsigned long address){
    unsigned long tag = address >> (s+b);
    unsigned long index = (address >> b) - (tag << s);
    
    ++time;

    for (int i = 0; i < E; i++){
        if (cache[index][i].v && cache[index][i].tag == tag){
            cache[index][i].lastUsed = time;
            ++hits;
            return;
        }
    }
    
    ++misses;

    int replaceBlock = LRU(index);

    if (cache[index][replaceBlock].v){
        ++envictions;
    }

    cache[index][replaceBlock].v = 1;
    cache[index][replaceBlock].tag = tag;
    cache[index][replaceBlock].lastUsed = time;
}

int main(int argc, char** argv) {
    int opt = 0;
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1){
        switch (opt){
            case 's':
                s = atoi(optarg);
                break;

            case 'E':
                E = atoi(optarg);
                break;

            case 'b':
                b = atoi(optarg);
                break;

            case 't':
                t = optarg;
                break;

            default:
                return 0;
        }
    }

    int row = 2 << (s - 1);
    initializeCache(row);

    char operation;
    unsigned long address;
    int  size;

    file = fopen(t, "r");
    while (fscanf(file, " %c %lx,%d", &operation, &address, &size) != EOF){
        /* load */
        if (operation == 'L'){
            accessCache(address); //load
        }
        /* modify (load and store) */
        else if (operation == 'M'){
            accessCache(address); //access
            accessCache(address); //modify
        }
        /* store */
        else if (operation == 'S'){
            accessCache(address);
        }
        else if (operation == 'I'){
            continue;
        }
        else break;
    }

    printSummary(hits, misses, envictions);
    fclose(file);
    
    for (int i = 0; i < row; i++){
        free(cache[i]);
    }
    free(cache);

    return 0;
}
