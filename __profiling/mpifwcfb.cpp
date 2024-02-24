#include<iostream>
#include<stdlib.h>
#include<limits.h>
#include<vector>
#include<deque>
#include<unordered_map>
#include<algorithm>
#include<mpi.h>
#include<string>

#define ON  1
#define OFF 0

#define LOGGING OFF

#define NVERT  5
#define INF   -1

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
    if(LOGGING || forced){
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

void print2d(int vec[NVERT][NVERT], int rowcount, int colcount, bool forced=false, std::string colsep=" ", std::string rowsep="\n"){
    if(LOGGING || forced){
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
    if(LOGGING || forced){
        for(int c=0; c<colcount; c++){
            std::cout << vec[c] << colsep;
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
}


int* solve_src(int nvertices, int* src_dest, int src_intr, int* intr_dest){
    for(int dest=0; dest<nvertices; dest++){
        if(intr_dest[dest] != INF){
            if(src_dest[dest] == INF)
                src_dest[dest] = src_intr + intr_dest[dest];
            else
                src_dest[dest] = std::min(src_intr+intr_dest[dest], src_dest[dest]);
        }
    }
    return src_dest;
}


// int* extract_col(int** mat, int col){
// }

void log(std::string msg){
    if(LOGGING) std::cout << msg << std::endl;
}

int main(int argc, char** argv){
    int root_rank = 0;
    int my_rank, nproc;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    std::string my_rank_s = std::to_string(my_rank);
    int *flatadj;
    int **adj;
    int nvertices = 0;
    if(my_rank == root_rank){
        std::cin >> nvertices;
    }
    // send nvertices to other processes
    MPI_Bcast(&nvertices, 1, MPI_INT, root_rank, MPI_COMM_WORLD);


    std::unordered_map<int, std::pair<int,int>> proc_rowrange;
    // if rows can be divided equally among the processes
    if(nvertices%nproc==0){
        int nrows_per_proc = nvertices/nproc;
        for(int rowno=0, procno=0; procno<nproc; rowno+=nrows_per_proc, procno++){
            proc_rowrange[procno] = std::make_pair(rowno, rowno+nrows_per_proc-1);
            if(my_rank == root_rank) log("from, to, nrows: " + std::to_string(proc_rowrange[procno].first) + " " + std::to_string(proc_rowrange[procno].second) + " " + std::to_string(proc_rowrange[procno].second-proc_rowrange[procno].first+1) + " ");
        }
    }
    else{
        int nrows_per_proc = nvertices/nproc;
        int residue = nvertices%nproc;
        int from =  0;
        int to   = -1;
        for(int rowno=0, procno=0; to!=nvertices-1; rowno+=nrows_per_proc, procno++){
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
                log("from, to, nrows: " + std::to_string(proc_rowrange[procno].first) + " " + std::to_string(proc_rowrange[procno].second) + " " + std::to_string(proc_rowrange[procno].second-proc_rowrange[procno].first+1) + " ");
            }
        }
    }


    int* src_dest;
    int src_intr;
    int* intr_dest;
    // alloc and take user inp in root proc
    if(my_rank == root_rank){
        adj  = alloc2d(nvertices, nvertices, uinp);
        print2d(adj, nvertices, nvertices);
    }
    // allocate only rows needed to process the slice of the grid
    else if(proc_rowrange.find(my_rank) != proc_rowrange.end()){
        int fromrow = proc_rowrange[my_rank].first;
        int torow   = proc_rowrange[my_rank].second;

        int nrows_to_alloc = torow-fromrow+1;
        flatadj = (int*)  calloc(sizeof(int), nrows_to_alloc*nvertices);
        adj     = (int**) calloc(sizeof(int*), nvertices);
        for(int r=fromrow, rflat=0; r<=torow; r++, rflat++){
            adj[r] = &(flatadj[rflat*nvertices]);
        }
        intr_dest = (int*) calloc(sizeof(int), nvertices);
    }




    if (proc_rowrange.find(my_rank) != proc_rowrange.end()){
        // log("mapsize: " + std::to_string(proc_rowrange.size()));
        log("processing procno: " + std::to_string(my_rank));
        for(int intr=0; intr<nvertices; intr++){
            log("--------------------------- intr: " + std::to_string(intr) + " --------------------------");
            log("r: " + my_rank_s);

            int fromrow = proc_rowrange[my_rank].first;
            int torow   = proc_rowrange[my_rank].second;

            // make root send rows to their corresponding processes
            if(my_rank == root_rank){
                for(int procno=root_rank+1; procno<nproc; procno++){
                    if(proc_rowrange.find(procno) == proc_rowrange.end()) continue;
                    int fromrow = proc_rowrange[procno].first;
                    int torow   = proc_rowrange[procno].second;
                    int nrows_to_send = torow-fromrow+1;
                    src_dest  = &adj[fromrow][0];
                    intr_dest = adj[intr];
                    MPI_Send(src_dest,  nrows_to_send*nvertices, MPI_INT, procno, procno, MPI_COMM_WORLD);
                    MPI_Send(intr_dest, nvertices, MPI_INT, procno, procno, MPI_COMM_WORLD);
                    log("sent: " + std::to_string(procno));
                }
                // process my rows
                for(int src=fromrow; src<=torow; src++){
                    src_dest  = adj[src];
                    src_intr  = adj[src][intr];
                    intr_dest = adj[intr];
                    if(src_intr != INF) solve_src(nvertices, src_dest, src_intr, intr_dest);
                }
            }


            // receive rows then wait for receive to complete one by one
            // , process as soon as receive complete, then send thr processed row back to root
            else{
                // receive rows
                log("r: " + my_rank_s + ":: receive rows");
                src_dest  = &adj[fromrow][0];
                int nrows_to_recv = torow-fromrow+1;
                MPI_Recv(src_dest, nrows_to_recv*nvertices, MPI_INT, root_rank, my_rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(intr_dest, nvertices, MPI_INT, root_rank, my_rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // process the received rows
                for(int src=fromrow; src<=torow; src++){
                    src_dest  = adj[src];
                    src_intr  = adj[src][intr];
                    if(src_intr != INF) solve_src(nvertices, src_dest, src_intr, intr_dest);
                    log("r: "+ my_rank_s + " processed src: " + std::to_string(src));
                }
                // send the processed rows back to root
                src_dest = &adj[fromrow][0];
                int nrows_to_send = torow-fromrow+1;
                MPI_Send(src_dest, nrows_to_send*nvertices, MPI_INT, root_rank, my_rank, MPI_COMM_WORLD);
            }


            // make root wait for all sends, then receive, and then wait for receives
            if(my_rank == root_rank){
                // receive rows from other processes
                // (and implicitly coalesce them to form the new adj)
                // NOTE: implicit because we are passing the pointers
                //       to the corresponding row of the adj matrix to the receive so no manual coalescing is needed
                for(int procno=root_rank+1; procno<nproc; procno++){
                    if(proc_rowrange.find(procno) == proc_rowrange.end()) continue;
                    int fromrow = proc_rowrange[procno].first;
                    int torow   = proc_rowrange[procno].second;
                    int nrows_to_recv = torow-fromrow+1;
                    src_dest = &adj[fromrow][0];
                    MPI_Recv(src_dest, nrows_to_recv*nvertices, MPI_INT, procno, procno, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    log("recv: " + std::to_string(procno));
                }

                log("before print2d");
                print2d(adj, nvertices, nvertices);
            }
        }
    }

    if(my_rank == root_rank) print2d(adj, nvertices, nvertices, true);

    // dealloc
    if(proc_rowrange.find(my_rank) != proc_rowrange.end()){
        int fromrow = proc_rowrange[my_rank].first;
        int torow   = proc_rowrange[my_rank].second;
        dealloc2d(adj, fromrow);
        if(my_rank != root_rank) free(intr_dest);
    }

    MPI_Finalize();
}

// run: mpic++ mpifwcfb.cpp -o mpifwcfb && mpiexec -n 5 ./mpifwcfb < fw.inp
// run: mpic++ mpifwcfb.cpp -o mpifwcfb && mpirun -n 12 --oversubscribe --use-hwthread-cpus ./mpifwcfb < fw.inp
