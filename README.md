# Multithreaded Graph Processing Program

## Overview
This program processes large text files containing graph data using multithreading. It provides the following functionalities:
- Reads a graph from a text file containing nodes and edges.
- Computes the total number of nodes and edges.
- Identifies the top 10 nodes with the most connections.
- Supports enabling or disabling thread affinity for performance optimization.

## Features
- **Multithreading**: Utilizes multiple threads to process the file in parallel.
- **Thread Affinity**: Option to bind threads to specific CPU cores for improved performance.
- **Large File Handling**: Capable of processing large graph files efficiently.
- **Customizable Threads**: Users can specify the number of threads.

## Usage
### Compilation
Use `gcc` with `-pthread` flag to compile the program:
```sh
gcc -o graph_processor graph_processor.c -pthread
```

### Running the Program
```sh
./graph_processor <filename> <num_threads> [affinity]
```
- `<filename>`: Path to the graph file.
- `<num_threads>`: Number of threads to use (up to 8 by default).
- `[affinity]`: Optional parameter to enable thread affinity.

### Example
```sh
./graph_processor graph.txt 4 affinity
```
This runs the program using 4 threads with thread affinity enabled.

## Input File Format
The program expects an edge list format, where each line represents an edge between two nodes:
```
1 2
2 3
3 4
4 5
```
Lines starting with `#` are treated as comments and ignored.

## Output
The program prints:
- The total number of unique nodes.
- The total number of edges.
- The top 10 nodes with the highest degrees.
- Execution time in seconds.

### Sample Output
```
Top 10 nodes with highest degree:
Node 5: 120 neighbors
Node 3: 110 neighbors
...

Total Unique Nodes: 50000
Total Edges: 250000

Execution Time: 1.235678 seconds
```

## Performance Considerations
- **Thread Affinity**: Binding threads to CPU cores can improve cache locality and performance.
- **Memory Usage**: The program allocates memory dynamically based on the largest node ID.
- **File Size**: Processing time depends on the file size and structure of the graph.

## Dependencies
- `gcc` (for compilation)
- `pthread` library (included in standard Linux distributions)



