#include <iostream>
#include <cstdio>
#include <cstdlib>

int main() {
    // Replace the command with the one you want to run
    const char* command = "mpic++ mpinq.cpp -o mpinq && { time mpirun -n 12 --use-hwthread-cpus --oversubscribe ./mpinq < nq.inp > nq.outp ; } 2>time.outp";


    // Open a pipe to capture the output of the command
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        std::cerr << "Error opening pipe." << std::endl;
        return EXIT_FAILURE;
    }

    // Read and print the command output
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::cout << buffer;
    }

    // Close the pipe
    if (pclose(pipe) == -1) {
        std::cerr << "Error closing pipe." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
