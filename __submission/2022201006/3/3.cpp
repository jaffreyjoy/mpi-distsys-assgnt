#include<iostream>
#include<stdlib.h>
#include<limits.h>
#include<vector>
#include<deque>
#include<unordered_map>
#include<algorithm>
#include<mpi.h>
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
    int* flatvec = (int*)  calloc(sizeof(int), rowcount*colcount);
    int** vec    = (int**) calloc(sizeof(int*), rowcount);
    for(int r=0; r<rowcount; r++){
        vec[r] = &(flatvec[r*colcount]);
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

void dealloc2d(int** vec, int freeindex){
    free(vec[freeindex]);
    free(vec);
}

void print2d(int** vec, int rowcount, int colcount, bool forced=false, std::string colsep=" ", std::string rowsep="\n"){
    if(LOGGING or forced){
    for(int r=0; r<rowcount; r++){
        for(int c=0; c<colcount; c++){
            std::cout << vec[r][c] << colsep;
        }
        std::cout << std::endl;
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
            std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
}

void print2dhslice(int** vec, int fromrow, int torow, int colcount, bool forced=false, std::string colsep=" ", std::string rowsep="\n"){
    if(LOGGING or forced){
        for(int r=fromrow; r<=torow; r++){
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

int alive_neighbors(int rowno, int colno, int lastrowind, int lastcolind, int** grid){
    int nalive = 0;

    bool isup     = (rowno>0);
    bool isdown   = (rowno<lastrowind);
    bool isleft   = (colno>0);
    bool isright  = (colno<lastcolind);

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
void simulate(int fromrow, int torow, int lastrowind, int lastcolind, int** grid, int** next_gen_grid){
    for(int r=fromrow; r<=torow; r++){
        for(int c=0; c<=lastcolind; c++){
            int nalive = alive_neighbors(r, c, lastrowind, lastcolind, grid);
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
}



int main(int argc, char** argv){
    int root_rank = 0;
    int my_rank, nproc;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int* flatgrid;
    int** grid;
    int** tgrid;
    int nrows = 0;
    int ncols = 0;
    int ngens = 0;

    bool transposed = false;

    if(my_rank == root_rank){
        std::cin >> nrows >> ncols >> ngens;
        if(ncols>nrows){
            transposed = true;
            std::swap(nrows, ncols);
        }
    }

    // send no. rows, cols and generations to other processes
    MPI_Bcast(&nrows, 1, MPI_INT, root_rank, MPI_COMM_WORLD);
    MPI_Bcast(&ncols, 1, MPI_INT, root_rank, MPI_COMM_WORLD);
    MPI_Bcast(&ngens, 1, MPI_INT, root_rank, MPI_COMM_WORLD);

    std::unordered_map<int, std::pair<int,int>> proc_rowrange;
    // if rows can be divided equally among the processes
    if(nrows%nproc==0){
        int nrows_per_proc = nrows/nproc;
        for(int rowno=0, procno=0; procno<nproc; rowno+=nrows_per_proc, procno++){
            proc_rowrange[procno] = std::make_pair(rowno, rowno+nrows_per_proc-1);
            if(my_rank == root_rank) log("from, to, nrows: " + std::to_string(proc_rowrange[procno].first) + " " + std::to_string(proc_rowrange[procno].second) + " " + std::to_string(proc_rowrange[procno].second-proc_rowrange[procno].first+1) + " ");
        }
    }
    else{
        int nrows_per_proc = nrows/nproc;
        int residue = nrows%nproc;
        int from =  0;
        int to   = -1;
        for(int rowno=0, procno=0; to!=nrows-1; rowno+=nrows_per_proc, procno++){
            from = to+1;
            if(residue>0){
                to = from+nrows_per_proc; residue--;
                proc_rowrange[procno] = std::make_pair(from, to);
            }
            else{
                to = from+nrows_per_proc-1;
                proc_rowrange[procno] = std::make_pair(from, to);
            }
            if(my_rank == root_rank) {
                log("procno: " + std::to_string(procno));
                log("to: " + std::to_string(to));
                log("nproc: " + std::to_string(nproc));
                log("nrows: " + std::to_string(nrows));
                log("ncols: " + std::to_string(ncols));
                log("mapsize: " + std::to_string(proc_rowrange.size()));
                log("from, to, nrows: " + std::to_string(proc_rowrange[procno].first) + " " + std::to_string(proc_rowrange[procno].second) + " " + std::to_string(proc_rowrange[procno].second-proc_rowrange[procno].first+1) + " ");
            }
        }
    }

    // alloc and take user inp in root proc
    if(my_rank == root_rank){
        if(transposed){
            grid  = alloc2d(ncols, nrows, uinp);
            print2d(grid, ncols, nrows);
            tgrid = alloc2d(nrows, ncols);
            for(int r=0; r<nrows; r++){
                for(int c=0; c<ncols; c++){
                    tgrid[r][c] = grid[c][r];
                }
            }
            dealloc2d(grid, 0);
            grid = tgrid;
            print2d(grid, nrows, ncols);
        }
        else{
            grid  = alloc2d(nrows, ncols, uinp);
            print2d(grid, nrows, ncols);
        }
    }
    // allocate only rows needed to process the slice of the grid
    else if(proc_rowrange.find(my_rank) != proc_rowrange.end()){
        int fromrow = proc_rowrange[my_rank].first;
        int torow   = proc_rowrange[my_rank].second;
        int from    = std::max(fromrow-1, 0);
        int to      = std::min(torow+1, nrows-1);

        int nrows_to_alloc = to-from+1;
        flatgrid = (int*)  calloc(sizeof(int), nrows_to_alloc*ncols);
        grid     = (int**) calloc(sizeof(int*), nrows);
        for(int r=from, rflat=0; r<=to; r++, rflat++){
            grid[r] = &(flatgrid[rflat*ncols]);
        }
    }


    int*  next_gen_flatgrid;
    int** next_gen_grid;
    int** prev_gen_grid;
    int waitforcount;
    MPI_Request* req;
    if (proc_rowrange.find(my_rank) != proc_rowrange.end()){
        log("mapsize: " + std::to_string(proc_rowrange.size()));
        log("processing procno: " + std::to_string(my_rank));
        for(int gen=0; gen<ngens; gen++){
            std::deque<MPI_Request*> sends;
            std::deque<MPI_Request*> receives;

            log("gen: " + std::to_string(gen));
            prev_gen_grid = grid;
            int fromrow = proc_rowrange[my_rank].first;
            int torow   = proc_rowrange[my_rank].second;
            int from    = std::max(fromrow-1, 0);
            int to      = std::min(torow+1, nrows-1);

            // allocate next gen grid and send rows to processes
            if(my_rank == root_rank){
                next_gen_grid = alloc2d(nrows, ncols);

                // send rows to other processes
                for(int procno=root_rank+1; procno<nproc; procno++){
                    if(proc_rowrange.find(procno) == proc_rowrange.end()) continue;
                    int fromrow = proc_rowrange[procno].first;
                    int torow   = proc_rowrange[procno].second;
                    // max 2 extra rows are sent as the topmost and bottommost rows would need
                    // the its own upper and lower rows respectively, to process the cells in their own rows
                    int from = std::max(fromrow-1, 0);
                    int to   = std::min(torow+1, nrows-1);
                    int nrows_to_send = to-from+1;
                    req = new MPI_Request;
                    sends.push_back(req);
                    // send
                    MPI_Isend(&prev_gen_grid[from][0], nrows_to_send*ncols, MPI_INT, procno, procno, MPI_COMM_WORLD, req);
                    // MPI_Send(&prev_gen_grid[from][0], nrows_to_send*ncols, MPI_INT, procno, procno, MPI_COMM_WORLD);
                    log("sent: " + std::to_string(procno));
                    print2dhslice(prev_gen_grid, from, to, ncols);
                }
            }
            // allocate next gen grid and receive rows from root
            else{
                int nrows_to_alloc = to-from+1;
                next_gen_flatgrid = (int*)  calloc(sizeof(int), nrows_to_alloc*ncols);
                next_gen_grid     = (int**) calloc(sizeof(int*), nrows);
                for(int r=from, rflat=0; r<=to; r++, rflat++){
                    next_gen_grid[r] = &(next_gen_flatgrid[rflat*ncols]);
                }

                // receive
                int nrows_to_receive = to-from+1;
                // req = new MPI_Request;
                MPI_Recv(&prev_gen_grid[from][0], nrows_to_receive*ncols, MPI_INT, root_rank, my_rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // MPI_Irecv(&prev_gen_grid[from][0], nrows_to_receive*ncols, MPI_INT, root_rank, my_rank, MPI_COMM_WORLD, req);
                // MPI_Wait(req, MPI_STATUS_IGNORE); free(req);
                log("receive: rank:: " + std::to_string(my_rank));
                print2dhslice(prev_gen_grid, from, to, ncols);
            }

            // do the job
            int lastrowind = to;
            int lastcolind = ncols-1;
            simulate(fromrow, torow, lastrowind, lastcolind, prev_gen_grid, next_gen_grid);
            log("simulate: rank:: " + std::to_string(my_rank));
            print2dhslice(next_gen_grid, fromrow, torow, ncols);


            // wait for sends, receive rows, wait for receives
            if(my_rank == root_rank){
                // wait for sends
                waitforcount = sends.size();
                for(int counter=0; counter<waitforcount; counter++){
                    req = sends.front(); sends.pop_front();
                    MPI_Wait(req, MPI_STATUS_IGNORE); free(req);
                }

                // receive rows from processes
                for(int procno=root_rank+1; procno<nproc; procno++){
                    if(proc_rowrange.find(procno) == proc_rowrange.end()) continue;
                    int fromrow = proc_rowrange[procno].first;
                    int torow   = proc_rowrange[procno].second;
                    int nrows_to_recv = torow-fromrow+1;
                    req = new MPI_Request;
                    receives.push_back(req);
                    // receive
                    MPI_Irecv(&next_gen_grid[fromrow][0], nrows_to_recv*ncols, MPI_INT, procno, procno, MPI_COMM_WORLD, req);
                    // MPI_Recv(&next_gen_grid[fromrow][0], nrows_to_recv*ncols, MPI_INT, procno, procno, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    log("receive: " + std::to_string(procno));
                    print2dhslice(next_gen_grid, fromrow, torow, ncols);
                }

                // wait for receives
                waitforcount = receives.size();
                for(int counter=0; counter<waitforcount; counter++){
                    req = receives.front(); receives.pop_front();
                    MPI_Wait(req, MPI_STATUS_IGNORE); free(req);
                }
            }
            // send processed slice back to root
            else{
                // send
                int nrows_to_send = torow-fromrow+1;
                // req = new MPI_Request;
                MPI_Send(&next_gen_grid[fromrow][0], nrows_to_send*ncols, MPI_INT, root_rank, my_rank, MPI_COMM_WORLD);
                // MPI_Isend(&next_gen_grid[fromrow][0], nrows_to_send*ncols, MPI_INT, root_rank, my_rank, MPI_COMM_WORLD, req);
                // MPI_Wait(req, MPI_STATUS_IGNORE); free(req);
            }

            dealloc2d(prev_gen_grid, from);
            grid = next_gen_grid;

            if(my_rank == root_rank) print2d(grid, nrows, ncols);
        }
    }

    if(my_rank == root_rank){
        if(transposed){
            std::swap(nrows, ncols);
            tgrid = alloc2d(nrows, ncols);
            for(int r=0; r<nrows; r++){
                for(int c=0; c<ncols; c++){
                    tgrid[r][c] = grid[c][r];
                }
            }
            dealloc2d(grid, 0);
            grid = tgrid;
        }
        print2d(grid, nrows, ncols, true);
    }

    // dealloc
    if(proc_rowrange.find(my_rank) != proc_rowrange.end()){
        int fromrow = proc_rowrange[my_rank].first;
        int torow   = proc_rowrange[my_rank].second;
        int from    = std::max(fromrow-1, 0);
        int to      = std::min(torow+1, nrows-1);
        dealloc2d(grid, from);
    }

    MPI_Finalize();
}

// run: mpic++ mpigol.cpp -o mpigol && mpiexec -n 5 --oversubscribe --use-hwthread-cpus ./mpigol < gol.inp > gol.outp
