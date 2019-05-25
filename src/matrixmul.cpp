#include "matrixmul.h"

namespace matrixmul {

AlgorithmCOLA::AlgorithmCOLA(std::unique_ptr<matrix::Sparse> matrixA, messaging::Communicator *communicator, int seed)
    : com{communicator} {
    // Parse input Sparse Matrix.
    int matrixN;
    std::vector<matrix::Sparse> matrixA_splitted;
    if (com->isCoordinator()) {
        matrixN = matrixA->n;
        matrixA_splitted = matrixA->Split(com->numProcesses());
    }

    com->BroadcastMatrixN(&matrixN);

    // Algorithm:
    // 1. Our implementation must start from a data distribution for c = 1 (i.e., as if there is no replication).
    //  Using a generator we supply, processes generate the dense matrix B in parallel (our generator is stateless,
    //  so it might be used in parallel by multiple MPI processes; however, each element of the matrix must be
    //  generated exactly once).
    matrixB = std::make_unique<matrix::Dense>(matrixN, com->rank(), com->numProcesses(), seed);

    if (com->isCoordinator()) {
        for (const auto &m : matrixA_splitted) {
            std::cout << m << std::endl;
        }
    }
}

void AlgorithmCOLA::replicate() {}

void AlgorithmCOLA::compute() {}

}