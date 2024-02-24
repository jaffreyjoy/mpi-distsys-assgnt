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
    int **adj;
    int nvertices = 0;
    if(my_rank == root_rank){
        std::cin >> nvertices;
    }
    // send nvertices to other processes
    MPI_Bcast(&nvertices, 1, MPI_INT, root_rank, MPI_COMM_WORLD);


    adj = alloc2d(nvertices, nvertices, uinp);
    // print2d(adj, nvertices, nvertices);
    // int adj[NVERT][NVERT] = {
    //      { 0,  4, -1,  5, -1}
    //     ,{-1,  0,  1, -1,  6}
    //     ,{ 2, -1,  0,  3, -1}
    //     ,{-1, -1,  1,  0,  2}
    //     ,{ 1, -1, -1,  4,  0}
    // };

    // get your job queue
    std::vector<int> my_srcs;
    for(int src=my_rank; src<nvertices; src+=nproc){
        my_srcs.push_back(src);
    }

    // vector [src] -> rank


    if(my_srcs.size() > 0){
        for(int intr=0; intr<nvertices; intr++){
            log("--------------------------- intr: " + std::to_string(intr) + " --------------------------");
            log("r: " + my_rank_s);
            int* src_dest;
            int src_intr;
            int* intr_dest;
            MPI_Request* req_sd;
            MPI_Request* req_id;

            std::deque<MPI_Request*> sends;
            std::deque<MPI_Request*> receives;

            // make root send rows to their corresponding processes
            if(my_rank == root_rank){
                for(int src=0; src<nvertices; src++){
                    int receiver_proc_rank = src%nproc;
                    if(receiver_proc_rank != root_rank){
                        req_sd  = new MPI_Request;
                        req_id  = new MPI_Request;
                        sends.push_back(req_sd );
                        sends.push_back(req_id );
                        src_dest  = adj[src];
                        intr_dest = adj[intr];
                        MPI_Isend(src_dest,  nvertices, MPI_INT, receiver_proc_rank, src, MPI_COMM_WORLD, req_sd);
                        MPI_Isend(intr_dest, nvertices, MPI_INT, receiver_proc_rank, src, MPI_COMM_WORLD, req_id);
                        log("sent: " + std::to_string(receiver_proc_rank));
                    }
                }
                // process my rows
                for(auto& src: my_srcs){
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
                for(int src: my_srcs){
                    req_sd = new MPI_Request;
                    req_id = new MPI_Request;
                    receives.push_back(req_sd);
                    receives.push_back(req_id);
                    MPI_Irecv(adj[src],  nvertices, MPI_INT, root_rank, src, MPI_COMM_WORLD, req_sd);
                    MPI_Irecv(adj[intr], nvertices, MPI_INT, root_rank, src, MPI_COMM_WORLD, req_id);
                }


                // wait for receive, process the row and send it back to root
                for(int src: my_srcs){
                    log("r: " + my_rank_s + ":: wait for receive");
                    // wait for the receive
                    req_sd = receives.front(); receives.pop_front();
                    req_id = receives.front(); receives.pop_front();
                    MPI_Wait(req_sd, MPI_STATUS_IGNORE); free(req_sd);
                    MPI_Wait(req_id, MPI_STATUS_IGNORE); free(req_id);
                    log("r: "+ my_rank_s); print1d(adj[src] , nvertices);
                    log("r: "+ my_rank_s); print1d(adj[intr], nvertices);

                    // process the received row
                    src_dest  = adj[src];
                    src_intr  = adj[src][intr];
                    intr_dest = adj[intr];
                    if(src_intr != INF) solve_src(nvertices, src_dest, src_intr, intr_dest);
                    log("r: "+ my_rank_s + " processed src: " + std::to_string(src)); print1d(src_dest, nvertices);
                    
                    // send the processed row back to root
                    req_sd = new MPI_Request;
                    sends.push_back(req_sd);
                    MPI_Isend(src_dest,  nvertices, MPI_INT, root_rank, src, MPI_COMM_WORLD, req_sd);
                }


                // wait for all sends
                for(int src: my_srcs){
                    req_sd = sends.front(); sends.pop_front();
                    MPI_Wait(req_sd, MPI_STATUS_IGNORE); free(req_sd);
                }

            }



            // make root wait for all sends, then receive, and then wait for receives
            if(my_rank == root_rank){
                // wait for sends
                int waitforcount = sends.size()/2;
                for(int counter=0; counter<waitforcount; counter++){
                    req_sd = sends.front(); sends.pop_front();
                    req_id = sends.front(); sends.pop_front();
                    MPI_Wait(req_sd, MPI_STATUS_IGNORE); free(req_sd);
                    MPI_Wait(req_id, MPI_STATUS_IGNORE); free(req_id);
                }

                // receive rows from other processes
                // (and implicitly coalesce them to form the new adj)
                // NOTE: implicit because we are passing the pointers
                //       to the corresponding row of the adj matrix to the receive so no manual coalescing is needed
                for(int src=0; src<nvertices; src++){
                    int receiver_proc_rank = src%nproc;
                    if(receiver_proc_rank != root_rank){
                        req_sd  = new MPI_Request;
                        receives.push_back(req_sd);
                        MPI_Irecv(adj[src], nvertices, MPI_INT, receiver_proc_rank, src, MPI_COMM_WORLD, req_sd);
                    }
                }

                // wait for all receives
                waitforcount = receives.size();
                for(int counter=0; counter<waitforcount; counter++){
                    req_sd = receives.front(); receives.pop_front();
                    MPI_Wait(req_sd, MPI_STATUS_IGNORE); free(req_sd);
                }

                // print2d(adj, nvertices, nvertices);
            }
        }
    }

    if(my_rank == root_rank) print2d(adj, nvertices, nvertices, true);

    MPI_Finalize();

    dealloc2d(adj, nvertices);
}

// run: mpic++ mpifw.cpp -o mpifw && mpiexec -n 5 ./mpifw < fw.inp
// run: mpic++ mpifw.cpp -o mpifw && mpirun -n 12 --use-hwthread-cpus --oversubscribe ./mpifw < fw.inp
