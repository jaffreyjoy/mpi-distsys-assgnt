import subprocess
import time
import argparse
import matplotlib.pyplot as plt
import os
import re

def write_n_to_file(n, filename="nq.inp"):
    with open(filename, "w") as f:
        f.write(f"{n}\n")

def profile_execution(n):
    # Compile the code
    compile_command = "mpic++ mpinq.cpp -o mpinq"
    os.system(compile_command)

    # Measure start time
    start_time = time.time()

    # Run the code
    run_command = f"mpirun -n {n} --use-hwthread-cpus --oversubscribe ./mpinq < nq.inp > nq.outp"
    os.system(run_command)

    # Measure end time
    end_time = time.time()

    return  end_time - start_time

def main():
    parser = argparse.ArgumentParser(description="Generate input file for N-Queens and profile MPI execution time.")
    parser.add_argument("-n", type=int, required=True, help="Number of queens (N)")
    args = parser.parse_args()

    write_n_to_file(args.n, "nq.inp")

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
    plt.title(f'MPI Execution Time vs Number of Processes  \n Board Size = {args.n}')
    # plt.show()

    # Save the plot as an image with the appropriate file name
    image_filename = "mpi_nq_execution_time_plot.png"
    plt.savefig(image_filename)
    print(f"Plot saved as {image_filename}")

if __name__ == "__main__":
    main()
