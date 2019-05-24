#ifndef UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
#define UW_MATRIX_MULTIPLICATION_MATRIXMUL_H

#include <vector>


enum Algorithms {
    COLA,   // 1.5D blocked column replicating A (ColA)
    COLABC, // 1.5D blocked column replicating all matrices (ColABC)
};

class MatrixCRS {
public:
    int n;
    std::vector<double> nonzero_values;
    std::vector<int> extents_of_rows;
    std::vector<int> column_indices;

    MatrixCRS(int n, std::vector<double> &&nonzero_values, std::vector<int> &&extents_of_rows,
        std::vector<int> &&column_indices);
};

#endif //UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
