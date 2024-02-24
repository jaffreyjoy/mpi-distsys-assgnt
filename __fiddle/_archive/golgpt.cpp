#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// Function to initialize the grid with random values
void initializeGrid(std::vector<std::vector<int>>& grid, int rows, int cols) {

    for (int i = 0; i < rows; ++i) {
        std::vector<int> row;
        for (int j = 0; j < cols; ++j) {
            int inp; std::cin >> inp;
            row.push_back(inp);
        }
        grid.push_back(row);
    }
}

// Function to print the grid
void printGrid(const std::vector<std::vector<int>>& grid) {
    for (const auto& row : grid) {
        for (int cell : row) {
            std::cout << cell << ' ';
        }
        std::cout << '\n';
    }
}

// Function to simulate one generation of the Game of Life
void simulateGeneration(std::vector<std::vector<int>>& grid, int rows, int cols) {
    std::vector<std::vector<int>> newGrid = grid;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int neighbors = 0;

            // Count live neighbors
            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {
                    int ni = (i + x + rows) % rows;
                    int nj = (j + y + cols) % cols;
                    neighbors += grid[ni][nj];
                }
            }

            // Exclude the current cell from the count
            neighbors -= grid[i][j];

            // Apply Conway's rules
            if (grid[i][j] == 1 && (neighbors < 2 || neighbors > 3)) {
                newGrid[i][j] = 0;  // Cell dies
            } else if (grid[i][j] == 0 && neighbors == 3) {
                newGrid[i][j] = 1;  // Cell becomes alive
            }
        }
    }

    // Update the original grid with the new generation
    grid = newGrid;
}

int main() {
    int rows, cols, generations;

    std::cin >> rows >> cols >> generations;

    // Create and initialize the grid
    std::vector<std::vector<int>> grid;
    initializeGrid(grid, rows, cols);

    // Simulate and display each generation
    for (int gen = 1; gen <= generations; ++gen) {
        simulateGeneration(grid, rows, cols);
    }

    // Display the final grid
    printGrid(grid);

    return 0;
}

// run: g++ golgpt.cpp -o golgpt && golgpt.exe < gol.inp > golgpt.outp
