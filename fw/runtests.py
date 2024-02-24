import os
import time

def run_tests(test_dir, executable):
    # Iterate over all .in files in the directory
    for filename in os.listdir(test_dir):
        if filename.endswith(".in"):
            # Extract filename without extension
            filename_no_ext = os.path.splitext(filename)[0]

            # Run the command with the input file
            command = f"mpic++ {executable}.cpp -o {executable} && mpirun -n 12 --oversubscribe --use-hwthread-cpus ./{executable} < {test_dir}/{filename} > {test_dir}/{filename_no_ext}.outx"
            # command = f"mpic++ {executable}.cpp -o {executable} && mpirun -n 12 ./{executable} < {test_dir}/{filename} > {test_dir}/{filename_no_ext}.outx"
            os.system(command)

            # Compare the output with the expected .out file
            outx_file = os.path.join(test_dir, f"{filename_no_ext}.outx")
            out_file = os.path.join(test_dir, f"{filename_no_ext}.out")
            if file_compare(outx_file, out_file):
                print(f"{filename_no_ext}: Output matches")
            else:
                print(f"{filename_no_ext}: Output differs")

def file_compare(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        # Remove trailing whitespaces and compare
        return f1.read().strip() == f2.read().strip()

if __name__ == "__main__":
    test_dir = "../test_cases/2"
    executable = "mpifw"
    run_tests(test_dir, executable)
