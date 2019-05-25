#ifndef UW_MATRIX_MULTIPLICATION_MATRIX_H
#define UW_MATRIX_MULTIPLICATION_MATRIX_H

#include <vector>
#include <iostream>
#include "densematgen.h"


class MatrixDense {
public:
    // MatrixDense contains a given number of full columns.
    int rows;           // Number of rows in the Matrix (=n).
    int column_base;    // First column saved in the Matrix.
    int columns;        // Number of columns saved in the Matrix.
    int columns_total;  // Total number of columns in the Matrix.
    std::vector<double> values;

    MatrixDense(int n, int part, int parts_total, int seed);
};

std::ostream& operator<<(std::ostream &os, const MatrixDense &m);

class MatrixSparse {
public:
    int n;

    std::vector<double> nonzero_values; // Values in the matrix.
    std::vector<int> extents_of_rows;   // Separation of values to different rows.
    std::vector<int> column_indices;    // Values' column indices.

    MatrixSparse(int n, std::vector<double> &&nonzero_values, std::vector<int> &&extents_of_rows,
                 std::vector<int> &&column_indices);
};

std::ostream& operator<<(std::ostream &os, const MatrixSparse &m);

#endif //UW_MATRIX_MULTIPLICATION_MATRIX_H
