#include <memory>
#include <mpi.h>
#include "parser.h"
#include "matrix.h"
#include "communicator.h"


int main(int argc, char **argv) {
    // Initialize communication between processes.
    auto communicator = messaging::Communicator(argc, argv);
    // Parse command line arguments.
    auto arg = parser::Arguments(argc, argv);
    // Parse provided sparse Matrix from plaintext file.
    std::unique_ptr<matrix::Sparse> matrix_sparse;
    if (communicator.isCoordinator()) {
        matrix_sparse = parser::parse_sparse_matrix(arg.sparse_matrix_file);
    }

    // Run algorithm.
    std::unique_ptr<matrixmul::Algorithm> algorithm;
    switch (arg.algorithm) {
        case matrixmul::Algorithms::COLA:
            algorithm = std::make_unique<matrixmul::AlgorithmCOLA>(std::move(matrix_sparse), &communicator, arg.seed);
            break;
        case matrixmul::Algorithms::COLABC:
            throw std::runtime_error("COLABC is not implemented yet.");
    }

    algorithm->replicate();
    algorithm->compute();

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
