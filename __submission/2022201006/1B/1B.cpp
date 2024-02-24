#include<iostream>
#include<stdlib.h>
#include<vector>
#include<mpi.h>

//  ♛

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


//            ---> r, c++
//            <--- r, c--
// ----------------------------
// |             |            |
// |             |            |
// |  (r-1,c-1)  |            |
// |             |            |
// |             |            |
// ----------------------------
// |             |            |
// |             |            |   | r++, c   ^
// |   (r,c-1)   |   (r,c)    |   v          | r--, c
// |             |            |
// |             |            |
// ----------------------------
// |             |            |
// |             |            |
// |  (r+1,c-1)  |            |
// |             |            |
// |             |            |
// ----------------------------

bool is_valid(int posr, int posc, int boardlen, int** board){

    // check top left diag
    for(int r=posr-1, c=posc-1; c>=0 && r>=0; r--, c--){
        if(board[r][c]) return false;
    }

    // check left row
    for(int c=posc-1; c>=0; c--){
        if(board[posr][c]) return false;
    }

    // check bottom left diag
    for(int r=posr+1, c=posc-1; c>=0 && r<boardlen; r++, c--){
        if(board[r][c]) return false;
    }

    return true;
}


void solve(int initposr, int initposc, int boardlen, int** board, int& ways){
    // std::cout << initposr << " " << initposc << std::endl;

    if(initposc==boardlen-1) {
        ways++;
        return;
    }

    // place on every row for next col
    for(int r=0, c=initposc+1; r<boardlen; r++){
        if(is_valid(r, c, boardlen, board)){
            // std::cout << "\t in: " << r << " " << c << std::endl;
            board[r][c] = 1;
            solve(r, c, boardlen, board, ways); // solve for further cols
            board[r][c] = 0;
        }
        else{
            // std::cout << "\t xn: " << r << " " << c << std::endl;
        }
    }
}


int main(int argc, char** argv) {
    int root_rank = 0;
    int my_rank, nproc;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int boardlen = 0;
    if(my_rank == root_rank){
        // std::cout << "n: "; std::cin >> boardlen;
        std::cin >> boardlen;
        // std::cout << argc << std::endl;
        // std::cout << argv[0] << std::endl;
    }

    // send boardlen to other processes
    MPI_Bcast(&boardlen, 1, MPI_INT, root_rank, MPI_COMM_WORLD);

    // get rows to be processed by running process
    std::vector<int> my_rows;
    for(int rowno=my_rank; rowno<boardlen; rowno+=nproc){
        my_rows.push_back(rowno);
    }

    int** board;
    int ways = 0;

    board = alloc2d(boardlen, boardlen, yes, 0);

    // place on every row for first col
    for(int r: my_rows){
        board[r][0] = 1;
        solve(r, 0, boardlen, board, ways); // solve for further cols
        board[r][0] = 0;
    }

    // std::cout << "ways: " << ways << std::endl;

    // calculate total ways
    int total_ways = 0;
    MPI_Reduce(&ways, &total_ways, 1, MPI_INT, MPI_SUM, root_rank, MPI_COMM_WORLD);
    if(my_rank == root_rank){
        // std::cout << "tways: " << total_ways << std::endl;
        std::cout << total_ways << std::endl;
    }

    dealloc2d(board, boardlen);

    MPI_Finalize();
}

// run: mpic++ mpinq.cpp -o mpinq && mpiexec -n 5 ./mpinq < nq.inp
// run: mpic++ mpinq.cpp -o mpinq && mpirun -n 12 --oversubscribe --use-hwthread-cpus ./mpinq < nq.inp