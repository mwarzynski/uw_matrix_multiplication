#include "matrix.h"

namespace matrix {

int block_column_size(int width, int blocks) {
    int columns = (width / blocks);
    if (width % blocks != 0) {
        columns++;
    }
    return columns;
}

int block_column_base(int width, int *block_width, int part) {
    int column_base = (*block_width) * part;
    if (column_base + (*block_width) >= width) {
        *block_width = width - column_base;
    }
    return column_base;
}

Dense::Dense(int n, int part, int parts_total, int seed) : rows{n}, columns_total{n} {
    columns = block_column_size(n, parts_total);
    column_base = block_column_base(n, &columns, part);
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < columns; c++) {
            values.push_back(generate_double(seed, r, column_base + c));
        }
    }
}

std::ostream &operator<<(std::ostream &os, const Dense &m) {
    int i = 0;
    for (int r = 0; r < m.rows; r++) {
        for (int c = 0; c < m.columns; c++) {
            os << "(" << r << "," << m.column_base + c << ")=";
            if (m.values[i] != 0) {
                os << m.values[i];
            } else {
                os << "0.000";
            }
            os << "\t";
            i++;
        }
        os << std::endl;
    }
    return os;
}

Sparse::Sparse(int n, std::vector<double> &&values, std::vector<int> &&rows_number_of_values,
               std::vector<int> &&values_column) : n{n}, values{values},
                                                    rows_number_of_values{rows_number_of_values},
                                                    values_column{values_column} {}

std::vector<Sparse> Sparse::Split(int p) {
    std::vector<std::vector<double>> m_values(p);
    std::vector<int> m_last_row(p);
    std::vector<std::vector<int>> m_rows_values(p);
    std::vector<std::vector<int>> m_value_column(p);

    int block_width = block_column_size(n, p);
    int it = 0;
    for (int row = 0; row < n; row++) {
        int values_in_row = rows_number_of_values[row + 1] - rows_number_of_values[row];
        for (int i = 0; i < values_in_row; i++) {
            int column = values_column[it];
            int part = column / block_width;
            if (row > m_last_row[part]) {
                m_rows_values[part].push_back(m_values[part].size());
                m_last_row[part] = row;
            }
            m_values[part].push_back(values[it]);
            m_value_column[part].push_back(column);
            it++;
        }
    }

    std::vector<Sparse> matrixes;
    for (int i = 0; i < p; i++) {
        auto m = Sparse(n, std::move(m_values[i]), std::move(m_rows_values[i]), std::move(m_value_column[i]));
        matrixes.push_back(m);
    }

    return matrixes;
}

std::ostream &operator<<(std::ostream &os, const Sparse &m) {
    int n = 0;
    for (int r = 0; r < m.n; r++) {
        int values_in_row = m.rows_number_of_values[r+1] - m.rows_number_of_values[r];
        for (int i = 0; i < m.n; i++) {
            if (values_in_row > 0 && i == m.values_column[n]) {
                os << m.values[n++];
                values_in_row--;
            } else {
                os << "0.00000";
            }
            os << "\t";
        }
        os << std::endl;
    }
    return os;
}

}
