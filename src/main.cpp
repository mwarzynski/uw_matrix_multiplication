#include <iostream>
#include <mpi.h>
#include "densematgen.h"
#include "parser.h"


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

    if (rank == 0) {
        MatrixCRS* matrix;
        try {
            matrix = parser::parse_sparse_matrix(arg->sparse_matrix_file);
        } catch (std::exception &e) {
            std::cout << "Parsing " << arg->sparse_matrix_file << ": " << e.what() << std::endl;
            MPI_Finalize();
            return 2;
        }
        std::cout << matrix->n << std::endl;
        for (double i : matrix->nonzero_values) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
