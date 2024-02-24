#include<iostream>
#include<stdlib.h>
#include<limits.h>
#include<vector>
#include<deque>
#include<unordered_map>
#include<algorithm>
// #include<mpi.h>
#include<string>

// #define and &&
// #define or  ||

#define ON  1
#define OFF 0

#define LOGGING OFF
#define NROWS  10
#define NCOLS  10

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

void print2d(int** vec, int rowcount, int colcount, bool forced=false, std::string colsep=" ", std::string rowsep="\n"){
    if(LOGGING or forced){
    for(int r=0; r<rowcount; r++){
        for(int c=0; c<colcount; c++){
            std::cout << vec[r][c] << colsep;
        }
        std::cout << rowsep;
    }
    std::cout << std::endl;
    std::cout << std::endl;
    }
}

void print2d(int vec[NROWS][NCOLS], int rowcount, int colcount, bool forced=false, std::string colsep=" ", std::string rowsep="\n"){
    if(LOGGING or forced){
        for(int r=0; r<rowcount; r++){
            for(int c=0; c<colcount; c++){
                std::cout << vec[r][c] << colsep;
            }
            std::cout << rowsep;
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
}

void print1d(int* vec, int colcount, bool forced=false, std::string colsep=" "){
    if(LOGGING or forced){
        for(int c=0; c<colcount; c++){
            std::cout << vec[c] << colsep;
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
}


// int* extract_col(int** mat, int col){
// }

void log(std::string msg){
    if(LOGGING) std::cout << msg << std::endl;
}

enum life{dead, alive};

int alive_neighbors(int rowno, int colno, int nrows, int ncols, int** grid){
    int nalive = 0;

    bool isup     = (rowno>0);
    bool isdown   = (rowno<nrows-1);
    bool isleft   = (colno>0);
    bool isright  = (colno<ncols-1);

    // up left
    if(isup and isleft)    nalive+=grid[rowno-1][colno-1];

    // up
    if(isup)               nalive+=grid[rowno-1][colno];

    // up right
    if(isup and isright)   nalive+=grid[rowno-1][colno+1];

    // left
    if(isleft)             nalive+=grid[rowno][colno-1];

    // right
    if(isright)            nalive+=grid[rowno][colno+1];

    // down left
    if(isdown and isleft)  nalive+=grid[rowno+1][colno-1];

    // down
    if(isdown)             nalive+=grid[rowno+1][colno];

    // down right
    if(isdown and isright) nalive+=grid[rowno+1][colno+1];

    return nalive;
}


/*
    https://playgameoflife.com/
    Rules:
        1. Any live cell with fewer than two live neighbours dies, as if by underpoulation.
        2. Any live cell with two or three live neighbours lives on to the next generation.
        3. Any live cell with more than three live neighbours dies, as if by overpopulation.
        4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
*/
int** simulate(int nrows, int ncols, int** grid){
    int** next_gen_grid = alloc2d(nrows, ncols);
    for(int r=0; r<nrows; r++){
        for(int c=0; c<ncols; c++){
            int nalive = alive_neighbors(r, c, nrows, ncols, grid);
            log("r c: " + std::to_string(r) + " " + std::to_string(c) + ":: " + std::to_string(nalive));
            int& current = grid[r][c];
            int& nextgen = next_gen_grid[r][c];
            if (current==alive and nalive < 2)
                nextgen = dead;
            else
            if (current==alive and (nalive==2 or nalive==3))
                nextgen = alive;
            else
            if (current==alive and nalive > 3)
                nextgen = dead;
            else
            if (current==dead and nalive==3)
                nextgen = alive;
            else
                nextgen = current;
        }
    }
    return next_gen_grid;
}



int main(){
    int **grid;
    int nrows = 0;
    int ncols = 0;
    int ngens = 0;

    std::cin >> nrows >> ncols >> ngens;
    grid = alloc2d(nrows, ncols, uinp);

    for(int gen=0; gen<ngens; gen++){
        log("gen: " + std::to_string(gen));
        int** prev_gen_grid = grid;
        grid = simulate(nrows, ncols, grid);
        dealloc2d(prev_gen_grid, nrows);
    }

    print2d(grid, nrows, ncols, true);

    dealloc2d(grid, nrows);
}

// run: g++ gol.cpp -o gol && gol.exe < gol.inp > gol.outp
