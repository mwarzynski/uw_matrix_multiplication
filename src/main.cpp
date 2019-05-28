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
        if (matrix_sparse->n % arg.replication_group_size != 0) {
            throw std::invalid_argument("n % c != 0");
        }
    }

    // Run algorithm.
    std::unique_ptr<matrixmul::Algorithm> algorithm;
    switch (arg.algorithm) {
        case matrixmul::Algorithms::COLA:
            algorithm = std::make_unique<matrixmul::AlgorithmCOLA>(std::move(matrix_sparse), &communicator, arg.replication_group_size, arg.seed);
            break;
        case matrixmul::Algorithms::COLABC:
            algorithm = std::make_unique<matrixmul::AlgorithmCOLABC>(std::move(matrix_sparse), &communicator, arg.seed);
            break;
    }

    // 2. After this initial data distribution, processes should contact their peers in replication groups and
    // exchange their parts of matrices.
    algorithm->phase_replication();

    // 3. Computation.
    algorithm->phase_computation(arg.exponent);

    // Final phase of gathering results from the workers.
    if (arg.ge_value > 0) {
        algorithm->phase_final_ge(arg.ge_value);
    } else if (arg.print_the_matrix_c) {
        algorithm->phase_final_matrix();
    }

    // Write two versions of your program. In a basic version, do not use any libraries for local (inside a process)
    // matrix multiplication. In a second version, use MKL for local matrix multiplication.

    return 0;
}
