#ifndef UW_MATRIX_MULTIPLICATION_MATRIX_H
#define UW_MATRIX_MULTIPLICATION_MATRIX_H

#include <memory>
#include <vector>
#include <iostream>
#include <cassert>
#include "densematgen.h"


namespace matrix {

class Dense {
public:
    int n_original;
    // MatrixDense contains a given number of full columns.
    int rows;           // Number of rows in the Matrix (=n).
    int column_base;    // First column saved in the Matrix.
    int columns;        // Number of columns saved in the Matrix.
    int columns_total;  // Total number of columns in the Matrix.
    std::vector<double> values;

    // Creates new Dense matrix filled with random values.
    Dense(int n, int n_original, int part, int parts_total, int seed);
    // Creates new Dense matrix filled with zeroes.
    Dense(int n, int n_original, int part, int parts_total);
    // Creates new Dense matrix based on provided values.
    Dense(int n, int n_original, int column_base, int columns, int columns_total, std::vector<double> &&values);
    // Creates new Dense matrix within provided column range filled with zeroes.
    Dense(int n, int n_original, std::pair<int, int> column_range);

    std::pair<int, int> ColumnRange();
    double Get(int x, int y);
    void Set(int x, int y, double value);
    void ItemAdd(int x, int y, double value);

private:
    size_t valuesIndex(int x, int y);
};

using Denses = std::vector<std::unique_ptr<Dense>>;

std::unique_ptr<Dense> Merge(Denses &&ds);

std::unique_ptr<Dense> MergeSame(Denses &&ds);

std::ostream& operator<<(std::ostream &os, const Dense &m);

class Sparse {
public:
    int n;

    std::vector<double> values;             // Values in the matrix.
    std::vector<int> rows_number_of_values; // Separation of values to different rows.
    std::vector<int> values_column;         // Values' column indices.

    // Creates new Sparse matrix based on provided values.
    Sparse(int n, std::vector<double> &&values, std::vector<int> &&rows_number_of_values,
                 std::vector<int> &&values_column);
    // Creates new Sparse matrix as a result from merging two provided ones.
    Sparse(Sparse *a, Sparse *b);

    // Splits the matrix into a 'processes' number of matrices. You may choose the dimension to split.
    std::vector<Sparse> Split(int processes, bool split_by_column);
};

std::ostream& operator<<(std::ostream &os, const Sparse &m);

class SparseIt {
public:
    explicit SparseIt(Sparse *m);

    std::tuple<int,int,double> Value(); // (x,y,value)
    bool Next();
private:
    Sparse *_m;
    int i = -1;
    int r = -1;
    int _values_in_row = 0;
};

}

#endif //UW_MATRIX_MULTIPLICATION_MATRIX_H
