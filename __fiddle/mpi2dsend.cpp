#include <stdio.h>
#include <mpi.h>

#define ROWS 4
#define COLS 4

void printArray(int array[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%d\t", array[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    int rank, size;
    int array[ROWS][COLS];
    int subArray[ROWS / 2][COLS];  // Each process gets a 2x4 subarray

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) {
            printf("This program requires exactly 2 processes\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        // Initialize the 2D array with dummy data
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                array[i][j] = i * COLS + j + 1;  // Adding 1 for dummy data
            }
        }

        printf("Original Array with Dummy Data:\n");
        printArray(array);

        // Send the first half of the array to process 1
        MPI_Send(&array[ROWS / 2][0], ROWS / 2 * COLS, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
        // Receive the first half of the array from process 0
        MPI_Recv(&subArray[0][0], ROWS / 2 * COLS, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Received Subarray in Process 1:\n");
        printArray(subArray);
    }

    MPI_Finalize();

    return 0;
}


// run: mpic++ mpi2dsend.cpp -o mpi2dsend && mpirun -n 2 ./mpi2dsend 