#ifndef UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
#define UW_MATRIX_MULTIPLICATION_MATRIXMUL_H

#include <vector>
#include <iostream>


enum Algorithms {
    COLA,   // 1.5D blocked column replicating A (ColA)
    COLABC, // 1.5D blocked column replicating all matrices (ColABC)
};

class MatrixCRS {
public:
    int n;

    std::vector<double> nonzero_values; // values in the matrix
    std::vector<int> extents_of_rows;   // separation of values to different rows
    std::vector<int> column_indices;    // values' column indices

    MatrixCRS(int n, std::vector<double> &&nonzero_values, std::vector<int> &&extents_of_rows,
        std::vector<int> &&column_indices);
};

std::ostream& operator<<(std::ostream &os, const MatrixCRS &m);

#endif //UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
