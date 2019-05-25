#ifndef UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H
#define UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H

#include "mpi.h"


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
};

}

#endif //UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H
