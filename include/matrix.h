#ifndef UW_MATRIX_MULTIPLICATION_MATRIX_H
#define UW_MATRIX_MULTIPLICATION_MATRIX_H

#include <vector>
#include <iostream>
#include "densematgen.h"

namespace matrix {

class Dense {
public:
    // MatrixDense contains a given number of full columns.
    int rows;           // Number of rows in the Matrix (=n).
    int column_base;    // First column saved in the Matrix.
    int columns;        // Number of columns saved in the Matrix.
    int columns_total;  // Total number of columns in the Matrix.
    std::vector<double> values;

    Dense(int n, int part, int parts_total, int seed);
};

std::ostream& operator<<(std::ostream &os, const Dense &m);

class Sparse {
public:
    int n;

    std::vector<double> values;             // Values in the matrix.
    std::vector<int> rows_number_of_values; // Separation of values to different rows.
    std::vector<int> values_column;         // Values' column indices.

    Sparse(int n, std::vector<double> &&values, std::vector<int> &&rows_number_of_values,
                 std::vector<int> &&values_column);

    std::vector<Sparse> Split(int p);
};

std::ostream& operator<<(std::ostream &os, const Sparse &m);

}

#endif //UW_MATRIX_MULTIPLICATION_MATRIX_H
