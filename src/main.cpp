#include <memory>
#include <mkl_spblas.h>
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

    auto matrixB = std::make_unique<matrix::Dense>(10, 10, 0, 1, 42);
    auto matrixC = std::make_unique<matrix::Dense>(10, 10, 0, 1);
    sparse_matrix_t matrixA;
    auto status = mkl_sparse_d_create_csr(
        &matrixA,                                        // matrix A to create
        SPARSE_INDEX_BASE_ZERO,                          // Indicates how input arrays are indexed
        matrix_sparse->n,                                // Number of rows of matrix A.
        matrix_sparse->n,                                // Number of columns of matrix A.
        matrix_sparse->rows_number_of_values.data(),     // rows_start, Array of length at least m: rows_start[i] - indexing
        matrix_sparse->rows_number_of_values.data() + 1, // rows_end, Array of at least length m  : rows_end[i] - indexing - 1
        matrix_sparse->values_column.data(),             // col_indx: for zero-based indexing, array containing the column indices for each non-zero element of the matrix A
        matrix_sparse->values.data());                   // values
    if (status != SPARSE_STATUS_SUCCESS) {
        std::cout << status << std::endl;
        return 2;
    }
    matrix_descr matrixDescrl{SPARSE_MATRIX_TYPE_GENERAL, SPARSE_FILL_MODE_LOWER, SPARSE_DIAG_NON_UNIT};

    // y := alpha*op(A)*x + beta*y
    auto m = mkl_sparse_d_mm(
        SPARSE_OPERATION_NON_TRANSPOSE, //
        1,                              // alpha
        matrixA,                        // A
        matrixDescrl,                   // A desc
        SPARSE_LAYOUT_ROW_MAJOR,        // layout (SPARSE_LAYOUT_COLUMN_MAJOR / SPARSE_LAYOUT_ROW_MAJOR)
        matrixB->values.data(),         // Array of size at least rows*cols.
        10,                             // columns
        10,                             // ldx (Specifies the leading dimension of matrix x.)
        1,                              // beta
        matrixC->values.data(),         // Array of size at least rows*cols
        10);                            // ldy
    if (m != SPARSE_STATUS_SUCCESS) {
        std::cout << m << std::endl;
        return 2;
    }

    std::cout << *matrix_sparse << std::endl;
    std::cout << *matrixB << std::endl;
    std::cout << *matrixC << std::endl;
    return 0;

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
