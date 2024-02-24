#include<iostream>
#include<stdlib.h>
#include<limits.h>
#include<vector>
#include<algorithm>
#include <thread> 

#define NVERT 5
#define INF -1

enum inittype {no, yes, uinp};

int** alloc2d(int rowcount, int colcount, enum inittype init=no, int initval=0){
    int** vec = (int**) calloc(sizeof(int*), rowcount);
    for(int r=0; r<rowcount; r++){
        vec[r] = (int*) calloc(sizeof(int), colcount);
    }
    if(init){
        int val = initval;
        for(int r=0; r<rowcount; r++){
            for(int c=0; c<colcount; c++){
                if(init == uinp) {
                    std:: cin >> val;
                }
                vec[r][c] = val;
            }
        }
    }
    return vec;
}

void dealloc2d(int** vec, int rowcount){
    for(int r=0; r<rowcount; r++){
        free(vec[r]);
    }
    free(vec);
}

void print2d(int** vec, int rowcount, int colcount, std::string colsep=" ", std::string rowsep="\n"){
    for(int r=0; r<rowcount; r++){
        for(int c=0; c<colcount; c++){
            std::cout << vec[r][c] << colsep;
        }
        std::cout << rowsep;
    }
}

void print2d(int vec[NVERT][NVERT], int rowcount, int colcount, std::string colsep=" ", std::string rowsep="\n"){
    for(int r=0; r<rowcount; r++){
        for(int c=0; c<colcount; c++){
            std::cout << vec[r][c] << colsep;
        }
        std::cout << rowsep;
    }
}


int* solve_src(int nvertices, int* src_dest, int src_intr, int* intr_dest){
    for(int dest=0; dest<nvertices; dest++){
        if(intr_dest[dest] != INF){
            if(src_dest[dest] == INF)
                src_dest[dest] = src_intr +  intr_dest[dest];
            else
                src_dest[dest] = std::min(src_intr+intr_dest[dest], src_dest[dest]);
        }
    }
    return src_dest;
}


// int* extract_col(int** mat, int col){
// }

int main(){
    int nvertices = NVERT;
    // int** adj = alloc2d(nvertices, nvertices, uinp);
    int adj[NVERT][NVERT] = {
         { 0,  4, -1,  5, -1}
        ,{-1,  0,  1, -1,  6}
        ,{ 2, -1,  0,  3, -1}
        ,{-1, -1,  1,  0,  2}
        ,{ 1, -1, -1,  4,  0}
    };

    int nproc = 3;
    std::vector<int> my_srcs;
    for(int src=0; src<nvertices; src+=nproc){
        my_srcs.push_back(src);
    }

    // vector [src] -> rank

    for(int intr=0; intr<nvertices; intr++){
        std::vector<std::thread*> srcthreads;
        for(int src=0; src<nvertices; src++){
            int* src_dest  = adj[src];
            int  src_intr  = adj[src][intr];
            if(src_intr != INF){
                int* intr_dest = adj[intr];
                // solve_src(nvertices, src_dest, src_intr, intr_dest);
                std::thread* t = new std::thread(solve_src, nvertices, src_dest, src_intr, intr_dest);
                srcthreads.push_back(t);
            }
        }
        for(auto& t: srcthreads) t->join();
    }

    print2d(adj, nvertices, nvertices);


    // dealloc2d(adj, nvertices);
}

// run: g++ floydwart.cpp -o floydwart && ./floydwart