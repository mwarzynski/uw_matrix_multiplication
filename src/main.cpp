#include <mpi.h>
#include "parser.h"
#include "matrix.h"
#include "communicator.h"


int main(int argc, char **argv) {
    auto communicator = messaging::Communicator(argc, argv);

    parser::Arguments *arg;
    try {
        arg = new parser::Arguments(argc, argv);
    } catch (std::exception &e) {
        std::cerr << "Arguments: " << e.what() << std::endl;
        return 1;
    }

    int n;
    MatrixSparse *matrix_sparse;
    if (communicator.isCoordinator()) {
        try {
            matrix_sparse = parser::parse_sparse_matrix(arg->sparse_matrix_file);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return 2;
        }
        n = matrix_sparse->n;
    }

    communicator.BroadcastMatrixN(&n);


    // Algorithm:
    // 1. Our implementation must start from a data distribution for c = 1 (i.e., as if there is no replication).
    //  Using a generator we supply, processes generate the dense matrix B in parallel (our generator is stateless,
    //  so it might be used in parallel by multiple MPI processes; however, each element of the matrix must be
    //  generated exactly once).
    auto matrix = MatrixDense(n, communicator.rank(), communicator.numProcesses(), arg->seed);
    std::cout << matrix;

    // 2. Process 0 loads the sparse matrix A from a CSR file (see bibliography for the description of the format) and
    //  then sends it to other processes. Each process should receive only a part of the matrix that it will store for
    //  data distribution for c = 1 (the coordinator should not send redundant data).
    //
    // Only after this initial data distribution, processes should contact their peers in replication groups and
    // exchange their parts of matrices.
    //
    // 3. Write two versions of your program. In a basic version, do not use any libraries for local (inside a process)
    //  matrix multiplication. In a second version, use MKL for local matrix multiplication.

    return 0;
}
