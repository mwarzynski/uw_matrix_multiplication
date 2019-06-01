#include <memory>
#include "parser.h"
#include "matrix.h"
#include "communicator.h"

/*
 * TODO: Things that are left to implement (WIP).
 * 4. Implement support for MKL matrix multiplication.
 * 5. Conduct performance tests (maybe tweak some things).
 */

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

    // 1. Initialize algorithm and data (with distribution).
    std::unique_ptr<matrixmul::Algorithm> algorithm;
    switch (arg.algorithm) {
        case matrixmul::Algorithms::COLA:
            algorithm = std::make_unique<matrixmul::AlgorithmCOLA>(std::move(matrix_sparse), &communicator,
                arg.replication_group_size, arg.seed);
            break;
        case matrixmul::Algorithms::COLABC:
            algorithm = std::make_unique<matrixmul::AlgorithmInnerABC>(std::move(matrix_sparse), &communicator,
                arg.replication_group_size, arg.seed);
            break;
    }

    // 2. After this initial data distribution, processes should contact their peers in replication groups and
    // exchange their parts of matrices.
    algorithm->phaseReplication();

    // 3. Computation.
    algorithm->phaseComputation(arg.exponent);

    // 4. Final phase of gathering results from the workers.
    if (arg.ge_value > 0) {
        algorithm->phaseFinalGE(arg.ge_value);
    } else if (arg.print_the_matrix_c) {
        algorithm->phaseFinalMatrix();
    }

    return 0;
}
