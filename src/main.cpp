#include <mpi.h>
#include "densematgen.h"
#include "parser.h"
#include "matrixmul.h"


int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int num_processes, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    parser::Arguments *arg;
    try {
        arg = new parser::Arguments(argc, argv);
    } catch (std::exception &e) {
        std::cout << "Arguments: " << e.what() << std::endl;
        MPI_Finalize();
        return 1;
    }

    // Algorithm:
    // 1. Our implementation must start from a data distribution for c = 1 (i.e., as if there is no replication).
    //  Using a generator we supply, processes generate the dense matrix B in parallel (our generator is stateless,
    //  so it might be used in parallel by multiple MPI processes; however, each element of the matrix must be
    //  generated exactly once).
    //
    // 2. Process 0 loads the sparse matrix A from a CSR file (see bibliography for the description of the format) and
    //  then sends it to other processes. Each process should receive only a part of the matrix that it will store for
    //  data distribution for c = 1 (the coordinator should not send redundant data).
    //
    // Only after this initial data distribution, processes should contact their peers in replication groups and
    // exchange their parts of matrices.
    //
    // Write two versions of your program. In a basic version, do not use any libraries for local (inside a process)
    // matrix multiplication. In a second version, use MKL for local matrix multiplication.

    MPI_Finalize();
    return 0;
}
