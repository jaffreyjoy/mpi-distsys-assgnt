import subprocess
import time
import argparse
import random
import matplotlib.pyplot as plt
import os
import re

def generate_grid(rows, cols):
    return [[random.choice([0, 1]) for _ in range(cols)] for _ in range(rows)]

def write_grid_to_file(grid, generations, filename="gol.inp"):
    with open(filename, "w") as f:
        f.write(f"{len(grid)} {len(grid[0])} {generations}\n")
        for row in grid:
            f.write(" ".join(map(str, row)) + "\n")


def profile_execution(n):
    # Compile the code
    compile_command = "mpic++ mpigol.cpp -o mpigol"
    os.system(compile_command)

    # Measure start time
    start_time = time.time()

    # Run the code
    run_command = f"mpirun -n {n} --use-hwthread-cpus --oversubscribe ./mpigol < gol.inp > gol.outp"
    os.system(run_command)

    # Measure end time
    end_time = time.time()

    return  end_time - start_time


def main():
    parser = argparse.ArgumentParser(description="Generate grid for Game of Life and profile MPI execution time.")
    parser.add_argument("-r", "--rows", type=int, required=True, help="Number of rows in the grid")
    parser.add_argument("-c", "--cols", type=int, required=True, help="Number of columns in the grid")
    parser.add_argument("-g", "--generations", type=int, required=True, help="Number of generations")
    args = parser.parse_args()

    grid = generate_grid(args.rows, args.cols)
    write_grid_to_file(grid, args.generations, "gol.inp")

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
    plt.title(f'MPI Execution Time vs Number of Processes  \n M={args.rows}, N={args.cols}, G={args.generations}')
    # plt.show()

    # Save the plot as an image with the appropriate file name
    image_filename = "mpi_gol_execution_time_plot.png"
    plt.savefig(image_filename)
    print(f"Plot saved as {image_filename}")

if __name__ == "__main__":
    main()
