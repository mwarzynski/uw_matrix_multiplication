# Communication-Avoiding Parallel Sparse-Dense Matrix Multiplication
The MPI Programming Assignment.

## Introduction

As in a typical supercomputer communication takes orders of magnitude more time than computation, there is a growing new field in high-performance algorithms: communication-avoidance. These algorithms trade communication for redundant computation or increased memory usage.

## Communication-Avoding Sparse-Dense Matrix Multiplication

Your goal is to implement two parallel algorithms for multiplying a sparse matrix A by a dense matrix B:
 - 1.5D Blocked Column A,
 - 1.5D Blocked Inner ABC
See the paper (website) for further details.

The main idea of both algorithms is to replicate (redundantly) the data, and then to do less communication rounds, than if the data were not replicated.
Both algorithms organize processes into "replication" groups of size `c` (`c` is a parameter of the algorithm).
The Column A algorithm replicates just the sparse matrix A. The Inner ABC algorithm replicates all the matrices: the result `C` is gathered from the parts stored in the replication group.

## Specific requirements

Your goal is to implement multiple matrix multiplication, i.e., compute (`C = A ( A ( A ... ( A B ) ) )`). You cannot change the order of multiplications.
The number of multiplications is given as a program parameter. Your implementation must start from a data distribution for `c = 1` (i.e., as if there is no replication).
Using a generator we supply, processes generate the dense matrix `B` in parallel (our generator is stateless, so it might be used in parallel by multiple MPI processes; however, each element of the matrix must be generated exactly once). Process `0` loads the sparse matrix `A` from a CSR file (see bibliography for the description of the format) and then sends it to other processes. Each process should receive only a part of the matrix that it will store for data distribution for `c = 1` (the coordinator should not send redundant data). Only after this initial data distribution, processes should contact their peers in replication groups and exchange their parts of matrices. Write two versions of your program.

In a basic version, do not use any libraries for local (inside a process) matrix multiplication.

In a second version, use **MKL** for local matrix multiplication. Assume that `A` and `B` are square matrices. Assume that, for performance testing, the number of rows and columns of `A` is much larger (at least thousands) than the number of MPI processes (at most hundreds). However, you cannot assume that the number of MPI processes divides the number of rows/columns (but you can extend matrices with `0` rows or columns, as long as they do not get printed in the final result).

## Input and output

Programs will be tested automatically. Please stick to the format below.

Your program will be run using the following instructions:
`cd xx123456; rm -rf build; mkdir build; cd build; cmake ..; make` `srun ./matrixmul -f sparse_matrix_file -s seed_for_dense_matrix -c repl_group_size -e exponent [-g ge_value] [-v] [-i] [-m]` where:
 - `sparse_matrix_file` is a CSR file storing the sparse matrix A. The first row contains 4 integers: the number of rows, the number of columns, the total number of non-zero elements, and the maximum number of non-zero elements in each row. The following 3 rows specify entries (array A in wikipedia's description); row offsets (array IA); and column indices (array JA). 
 - `-i` toggles the Inner algorithm (otherwise the Column algorithm is used); 
 - `-v` prints the matrix C (the multiplication result) in the row-major order: the first line specifies the number of rows and the number of columns of the result; i+1-th line is the i-th row of the matrix; 
 - `-c repl_group_size` specifies the number of processes in each replication group (i.e., parameter c from Koanantakool et al); 
 - `-e exponent` the number of multiplications to do, e.g., for `-e 3` your program should compute (`C = A ( A ( A B ) )`)
 - `-g ge_value` prints the number of elements in C greater than or equal to the `ge_value`. 
 - `-m` turns on MKL for in-process sparse-dense matrix multiplication (it's off by default). Do not print anything other than the matrix C (if `-v` is used) or a single integer (if `-g` is used) on stdout.

## Solution content

Please send us a single `.zip` file containing a single directory with your login (`ab123456`); the directory has at least the following files: * `densematgen.h`: our generator. This file cannot be modified. * `densematgen.cpp`: our generator. This file cannot be modified. For tests, we might use a different implementation of the generator (but it will be stateless); * `report.pdf`: a report describing your implementation. Estimate the numerical intensity of the problem (as in the roofline model). Describe the optimizations you implemented. Show weak and strong scaling results. For scaling, find instances and input parameters so that the measurements are realistic, but the (wall clock) run time does not exceed 3 minutes when more than 4 nodes are used.

## Scoring

 - correct MPI implementation of the Inner algorithm: 6 points; 
 - correct MPI implementation of the Column algorithm: 6 points; 
 - report: 4 points (incorrect implementations do not get these points); 
 - performance: 9 points (incorrect implementations do not get these points). We will score correctness on our test data; we take into account the floating point errors. If your solution passes most of the tests, but fails on some, we will contact you and you will be able to submit a patched version. We will use Okeanos for performance testing. To optimize performance, consider using advanced MPI operations, like asynchronous messages, collectives, custom datatypes, custom communicators (hint: for communicating inside a replication group). Remember to test the MKL library. You may also consider using OpenMP. Our performance tests will use `--tasks-per-node 24` unless you write in your report that your solution is more efficient with other value (e.g., you use MPI+OpenMP and you just need `--tasks-per-node 1` or `--tasks-per-node 2`). Please do submit your assignment by the due date. If you're late, your score will be reduced by 1 point for every 12 hours (i.e.: if you're late by 2h, we subtract 1 point from your score; if you're late by 25h, we subtract 3 points). There is a second due date - 13th June. Submitting by this due date is very risky. First, very good solutions submitted by this due date receive minimum passing score (i.e.: at most 10 points). Second, there will be less time for patching. Please do submit your assignment by the normal, first due date.

## Additional materials

Please do not use any source codes of matrix multiplication programs. We recommend reading the following documents: 
 - P.Koanantakool et al, Communication-Avoiding Parallel Sparse-Dense Matrix-Matrix Multiplication, IPDPS 2016, [http://www.eecs.berkeley.edu/~penpornk/spdm3_ipdps16.pdf](http://www.eecs.berkeley.edu/~penpornk/spdm3_ipdps16.pdf) 
 - Jesper Larsson Traff, William D. Gropp, and Rajeev Thakur, Self-Consistent MPI Performance Guidelines [http://www-unix.mcs.anl.gov/~thakur/papers/tpds-consistency.pdf](http://www-unix.mcs.anl.gov/~thakur/papers/tpds-consistency.pdf) 
 - CSR matrix format: [https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_.28CSR.2C_CRS_or_Yale_format.29](https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_.28CSR.2C_CRS_or_Yale_format.29) 

## FAQ

 - What language can I use? C or C++.
 - What datatype should I use for computation? Use doubles.
 - Do you really want us to, first, scatter A into as many unique pieces as there are processes, and only after that (if (c>1)) construct redundant A parts by communicating among processes? Why can't the coordinator send A partitioned for correct c? Matrix multiplication (and other distributed) algorithms commonly assume that the data is partioned evenly among processes before the algorithm starts (see the first lecture on alpha-beta efficiency model). The communication-avoiding algorithms reorganize data and thus add new communication (before the multiplication really begins). We want to be able to measure the time needed for this additional communication, which is possible only when the algorithm firstly distributes the data for c=1 (thus, as if there was no communication-avoidance), and only after that reorganizes the data.
 - Can process 0 load the whole sparse matrix A, and only after that partition it and send parts to other processes? Yes it can.
 - What is the c parameter? Number of processes in the replication group, or number of replication groups? And what is a replication group anyway? In this document, a replication group is a group of processes storing the same data (after replication phase, before the computation phase). In the replication phase, each process contacts ((c-1)) other processes; these processes combine the data they got from the initial data distribution.
 - Can I assume that the resulting C will fit in the memory of a single node? In general: if `-g` or no output option is used, you can't assume the result will fit in a single node. But you can assume that if `-v` is used, C will fit in a single node.
 - Can I assume that, when InnerABC is used, (p) is a multiply of (c^2) Yes, you can.
 - How should the processes know the size of matrix A? Process 0 can broadcast the size of A. In general, you can add messages to your implementation.
 - How are the example tests organized? `result_X_000Y_Z_A` is a result of multiplying `sparse05_000Y_Z` with `matrix01_000Y_A` X times (i.e., with `-e X`). For instance, in `result_2_00010_002_00291`, we multiply `sparse05_00010_002` with `matrix01_00010_291` and then mutliply the result again by `sparse05_00010_002`, or, in other words, `./matrixmul -f sparse05_0010_002 -s 291 -e 2 -v`.
 - Can a process temporary store a few (two, three) blocks of matrix A (e.g., when the matrix is shifted)? Yes, it can.
 - How should the output be formatted? See the description of `-v` in the Input/Output section. Additional instructions: white-spaces are not important (values can be separated by a single space); field formatting is not important; you should use a standard format for floating point numbers (eg. `12345.67890`) with at least 5 numbers after the dot.
 - Can I destroy B during computation? Yes, you can (as long as the result is correct).
 - In Inner, at the end, should the processes send their parts of matrix to process in layer 0, or can they directly send the parts to the coordinator (or just the counts for `-g`)? For fair measurements, please reduce first to layer 0, and only then send to the coordinator.
 - Can the coordinator allocate memory for 2|A|? Yes, it can.
 - Can we assume that the cluster nodes will use the same binary representation of numbers? Yes, you can. 
 - Can I use a different matrix representation for the sparse matrix A? The input is in CSR, but you can use any internal (i.e., in memory) representation, including CSR. The representation has to be sparse, i.e., A cannot use (O(n^2)) memory.

Author: Krzysztof Rzadca Created: 2019-05-14 Tue 15:30 [Emacs](http://www.gnu.org/software/emacs/) 25.3.50.1 ([Org](http://orgmode.org) mode 8.2.10) [Validate](http://validator.w3.org/check?uri=referer)
