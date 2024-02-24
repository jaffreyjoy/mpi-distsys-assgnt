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

// Function to count live neighbors for a given cell
int countLiveNeighbors(const std::vector<std::vector<int>>& grid, int i, int j, int rows, int cols) {
    int neighbors = 0;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            if (x == 0 && y == 0) continue;  // Exclude the current cell
            int ni = i + x;
            int nj = j + y;

            // Check boundaries
            if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                neighbors += grid[ni][nj];
            }
        }
    }

    return neighbors;
}

// Function to simulate one generation of the Game of Life
void simulateGeneration(std::vector<std::vector<int>>& grid, int rows, int cols) {
    std::vector<std::vector<int>> newGrid = grid;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int neighbors = countLiveNeighbors(grid, i, j, rows, cols);

            // Apply the specified rules
            if (grid[i][j] == 1) {
                if (neighbors < 2 || neighbors > 3) {
                    newGrid[i][j] = 0;  // Rule 1 and 3
                }
            } else {
                if (neighbors == 3) {
                    newGrid[i][j] = 1;  // Rule 4
                }
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


// run: g++ golgptx.cpp -o golgptx && golgptx.exe < gol.inp > golgptx.outp
