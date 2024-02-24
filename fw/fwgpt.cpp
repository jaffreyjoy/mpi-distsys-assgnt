#include <iostream>
#include <vector>

using namespace std;

const int INF = 1e9; // Assuming INF as a large value representing infinity

void floydWarshall(vector<vector<int>>& graph, int V) {
    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < V; ++i) {
            for (int j = 0; j < V; ++j) {
                if (graph[i][k] != INF && graph[k][j] != INF && graph[i][k] + graph[k][j] < graph[i][j]) {
                    graph[i][j] = graph[i][k] + graph[k][j];
                }
            }
        }
    }
}

int main() {
    int V;
    
    // Taking the number of vertices as input
    // cout << "Enter the number of vertices: ";
    cin >> V;

    // Taking the adjacency matrix as input
    vector<vector<int>> graph(V, vector<int>(V));
    // cout << "Enter the adjacency matrix (" << V << "x" << V << "):" << endl;
    for (int i = 0; i < V; ++i) {
        for (int j = 0; j < V; ++j) {
            cin >> graph[i][j];
            if (graph[i][j] == -1) { // Assuming -1 as infinity
                graph[i][j] = INF;
            }
        }
    }

    // Applying Floyd Warshall algorithm
    floydWarshall(graph, V);

    // Printing the resulting adjacency matrix without comma separation
    // cout << "Resulting adjacency matrix:" << endl;
    for (int i = 0; i < V; ++i) {
        for (int j = 0; j < V; ++j) {
            cout << graph[i][j] << ' ';
        }
        cout << endl;
    }

    return 0;
}


// run: g++ fwgpt.cpp -o fwgpt && ./fwgpt < fw1.inp > fw1gpt.outp