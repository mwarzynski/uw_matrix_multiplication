#ifndef UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H
#define UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H

#include <memory>
#include "mpi.h"
#include "matrix.h"

#define TAG_INITIALIZATION 10


namespace messaging {

class Communicator {
private:
    int _rank;
    int _num_processes;
public:

    Communicator(int argc, char **argv);
    ~Communicator();

    bool isCoordinator();
    static int rankCoordinator();
    int rank();
    int numProcesses();

    void BroadcastN(int n);
    int ReceiveN();

    void SendDense(matrix::Dense *m, int receiver, int tag);
    std::unique_ptr<matrix::Dense> ReceiveDense(int sender, int tag);

    void SendSparse(matrix::Sparse *m, int receiver, int tag);
    std::unique_ptr<matrix::Sparse> ReceiveSparse(int sender, int tag);
};

}

#endif //UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H
