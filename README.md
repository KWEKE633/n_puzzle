# n_puzzle

## N-Puzzle Solver
This project is a high-performance C++ implementation of an N-Puzzle (e.g., 8-puzzle, 15-puzzle) solver. It utilizes the A* (A-Star) search algorithm, aggressively optimized for memory locality and execution speed through a custom memory management architecture.

## Features & Core Optimizations
A Search Algorithm:* Guarantees the shortest path (optimal solution) when using an admissible heuristic.Custom MemoryPool: Pre-allocates a contiguous block of memory (std::vector::reserve) to store all generated nodes. This eliminates the massive overhead of dynamic allocation (new/malloc) during the search and maximizes CPU cache hit rates via spatial locality.Index-Based Node Tree: Nodes are linked using 32-bit integer indices rather than raw memory pointers. This ensures 100% safety against dangling pointers even if the underlying memory pool undergoes dynamic reallocation.Optimized Data Structures: Utilizes std::priority_queue for $O(\log N)$ extraction of the most promising nodes (Open Set) and std::unordered_set with a custom high-speed hash function for $O(1)$ state duplicate detection (Closed Set).1D Array State Representation: Multi-dimensional grid coordinates are calculated mathematically ($y = i / N$, $x = i \% N$) on a flat 1D array to further prevent cache fragmentation.

## Goal State (Snail Pattern)
Unlike the standard N-puzzle where the goal is a simple row-by-row sequence, the target solution for this solver is a spiral (snail) pattern. The numbers increment in a clockwise spiral towards the center, with the empty tile 0 placed at the very end of the sequence.

Example of a 3x3 Goal State:
```Plaintest
1 2 3
8 0 4
7 6 5
```

Example of a 4x4 Goal State:
```Plaintext
1  2  3  4
12 13 14  5
11  0 15  6
10  9  8  7
```

## PrerequisitesCompiler: 
C++20 compliant compiler (g++ or clang++).

```Bash
## Build Tool: makeBuild & Execution
# Compile the project
make

# Remove object files
make clean

# Remove object files and the executable
make fclean
```

## Usage
Run the executable by passing the puzzle map file as the primary argument. You can modify the search behavior using optional flags for heuristics and weights.

```Bash
./n_puzzle [options] <map_file>
```

## Command-Line Options
Flag,Description,Default
"-m, --manhattan",Uses Manhattan Distance as the heuristic. Guarantees optimal path.,Yes
"-u, --misplaced",Uses Misplaced Tiles (Hamming Distance) as the heuristic.,No
"-w <float>, --weight <float>",Applies a multiplier to the heuristic (Weighted A*). Sacrifices path optimality for significantly faster execution times on massive boards (N≥5).,1.0

## Examples

```Bash
# Standard execution (Manhattan Distance, Weight 1.0)
./n_puzzle map/valid/solvable_3.txt

# Fast execution using Weighted A* (Weight 5.0)
./n_puzzle -m -g map/valid/solvable_4.txt
```

## Input File Format
The solver accepts text files where the first line dictates the grid size ($N$), followed by an $N \times N$ grid of numbers. The number 0 represents the empty sliding tile. Comments can be prefixed with #.

```Plaintext
# Example of a 3x3 puzzle
3
3 2 4
5 7 1
6 8 0
```

## Performance Metrics Output
MetricDefinitionImplementation SourceTime ComplexityTotal number of states selected and expanded during the search.closed_set.size()Size ComplexityMaximum number of states held in memory at the peak of the search.MemoryPool::size()Path CostTotal number of moves required to reach the goal state.Reconstructed path length
