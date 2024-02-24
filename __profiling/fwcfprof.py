import subprocess
import time
import argparse
import random
import matplotlib.pyplot as plt
import re
import os

def generate_adj_matrix(n):
    adj_matrix = [[0] * n for _ in range(n)]
    for i in range(n):
        for j in range(n):
            if i != j:
                # Fill cells with random positive values or -1 (indicating infinity)
                adj_matrix[i][j] = random.choice([random.randint(1, 100), -1])
    return adj_matrix

def write_matrix_to_file(matrix, filename="fw.inp"):
    with open(filename, "w") as f:
        f.write(f"{len(matrix)}\n")
        for row in matrix:
            f.write(" ".join(map(str, row)) + "\n")

def profile_execution(n):
    # Compile the code
    compile_command = "mpic++ mpifwcf.cpp -o mpifwcf"
    os.system(compile_command)

    # Measure start time
    start_time = time.time()

    # Run the code
    run_command = f"mpirun -n {n} --use-hwthread-cpus --oversubscribe ./mpifwcf < fw.inp > fw.outp"
    os.system(run_command)

    # Measure end time
    end_time = time.time()

    return  end_time - start_time


def main():
    parser = argparse.ArgumentParser(description="Generate adjacency matrix for directed graph and profile MPI execution time.")
    parser.add_argument("-n", type=int, required=True, help="Number of vertices")
    args = parser.parse_args()

    n_vertices = args.n

    # adj_matrix = generate_adj_matrix(n_vertices)
    # write_matrix_to_file(adj_matrix)

    time_data = {}

    for n in range(1, 13):
        exec_time = profile_execution(n)
        time_data[n] = exec_time

        # Output profiling results for each n
        print(f"Profiling for n={n}:")
        print(f"User Time: {exec_time:.2f} seconds")

    # Plotting the results
    plt.plot(list(time_data.keys()), list(time_data.values()), marker='o')
    plt.xlabel('Number of Processes (n)')
    plt.ylabel('Total Execution Time (s)')
    plt.title(f'MPI Execution Time vs Number of Processes \n No. of vertices = {n_vertices}')
    # plt.show()

    # Save the plot as an image with the appropriate file name
    image_filename = "mpi_fwcf_execution_time_plot.png"
    plt.savefig(image_filename)
    print(f"Plot saved as {image_filename}")

if __name__ == "__main__":
    main()
