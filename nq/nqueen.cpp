#include<iostream>
#include<stdlib.h>

//  ♛


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


int main(){
    int boardlen = 14;
    int** board;
    int ways = 0;

    // <board>:: alloc
    board = (int**) calloc(sizeof(int*), boardlen);
    for(int i=0; i<boardlen; i++){
        board[i] = (int*) calloc(sizeof(int), boardlen);
    }
    // <board>:: init
    for(int r=0; r<boardlen; r++){
        for(int c=0; c<boardlen; c++){
            board[r][c] = 0;
        }
    }

    // place on every row for first col
    for(int r=0; r<boardlen; r++){
        board[r][0] = 1;
        solve(r, 0, boardlen, board, ways); // solve for further cols
        board[r][0] = 0;
    }

    std::cout << "ways: " << ways << std::endl;

    // <board>:: dealloc
    for(int i=0; i<boardlen; i++){
        free(board[i]);
    }
    free(board);

}

// run: g++ nqueen.cpp -o nqueen && ./nqueen