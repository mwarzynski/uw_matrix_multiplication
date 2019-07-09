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

Dense::Dense(int n, int n_original, int part, int parts_total, int seed) : n_original{n_original}, rows{n},
    columns_total{n} {
    columns = block_column_size(n, parts_total);
    column_base = block_column_base(n, &columns, part);
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < columns; c++) {
            if (c >= n_original)
                values.push_back(0);
            else
                values.push_back(generate_double(seed, r, column_base + c));
        }
    }
}

Dense::Dense(int n, int n_original, int part, int parts_total) : n_original{n_original}, rows{n}, columns_total{n} {
    columns = block_column_size(n, parts_total);
    column_base = block_column_base(n, &columns, part);
    if (columns < 0) {
        columns = 0;
        column_base = n;
        return;
    }
    size_t s = columns * n;
    assert(s < values.max_size());
    values.resize(columns * n);
}

Dense::Dense(int n, int n_original, int column_base, int columns, int columns_total, std::vector<double> &&values) :
    n_original{n_original}, rows{n}, column_base{column_base}, columns{columns}, columns_total{columns_total},
    values{values} {}

Dense::Dense(int n, int n_original, std::pair<int, int> column_range) : n_original{n_original}, rows{n}, columns_total{n} {
    column_base = column_range.first;
    columns = column_range.second - column_range.first;
    int size = columns * rows;
    if (size < 0) {
        size = 0;
    }
    values.resize(size);
}

std::pair<int, int> Dense::ColumnRange() {
    return std::make_pair(column_base, column_base + columns);
}

size_t Dense::valuesIndex(int x, int y) {
    int ry = y * columns;
    int rx = x - column_base;
    assert(ry + rx >= 0);
    assert(ry + rx < static_cast<int>(values.size()));
    return ry + rx;
}

double Dense::Get(int x, int y) {
    return values[valuesIndex(x, y)];
}

void Dense::Set(int x, int y, double value) {
    values[valuesIndex(x, y)] = value;
}

void Dense::ItemAdd(int x, int y, double value) {
    Set(x, y, Get(x, y) + value);
}

std::unique_ptr<Dense> MergeSame(Denses &&ds) {
    if (ds.empty()) {
        return nullptr;
    }
    if (ds.size() == 1) {
        return std::move(ds[0]);
    }

    int n = ds[0]->rows;
    int column_base = ds[0]->column_base;
    int columns = ds[0]->columns;
    for (size_t i = 0; i < ds.size() - 1; i++) {
        assert(ds[i+1]->column_base == ds[i]->column_base);
        assert(ds[i+1]->columns == ds[i]->columns);
    }
    size_t values_size = columns * n;
    std::vector<double> values;
    assert(values_size < values.max_size());
    values.resize(values_size);

    for (int r = 0; r < n; r++) {
        size_t it = (r * columns);
        for (int j = 0; j < columns; j++) {
            for (const auto &m : ds) {
                if (m->values[it + j] == 0)
                    continue;
                values[it + j] = m->values[it + j];
            }
        }
    }

    return std::make_unique<Dense>(n, ds[0]->n_original, column_base, columns, n, std::move(values));
}

std::unique_ptr<Dense> Merge(Denses &&ds) {
    if (ds.empty()) {
        return nullptr;
    }
    if (ds.size() == 1) {
        return std::move(ds[0]);
    }

    if (ds[0]->column_base == ds[1]->column_base) {
        return MergeSame(std::move(ds));
    }

    // Prepare meta information.
    int n = ds[0]->rows;
    int column_base = ds[0]->column_base;
    for (size_t i = 0; i < ds.size() - 1; i++) {
        if (ds[i]->columns <= 0 || ds[i+1]->column_base >= ds[i]->n_original) {
            continue;
        }
        if (ds[i+1]->column_base != ds[i]->column_base + ds[i]->columns) {
            std::cerr << ds[i+1]->column_base << " " << ds[i]->column_base << " " << ds[i]->columns << std::endl;
        }
        assert(ds[i+1]->column_base == ds[i]->column_base + ds[i]->columns);
    }
    int columns = 0;
    for (const auto &m : ds) {
        if (m->columns < 0) {
            continue;
        }
        columns += m->columns;
    }
    size_t values_size = columns * n;
    // Copy item values.
    std::vector<double> values;
    assert(values_size < values.max_size());
    values.resize(values_size);

    int i = 0;
    for (int r = 0; r < n; r++) {
        for (const auto &m : ds) {
            size_t base = (r * m->columns);
            for (int j = 0; j < m->columns; j++) {
                values[i++] = m->values[base + j];
            }
        }
    }

    // Create unique pointer to the newly created Matrix.
    return std::make_unique<Dense>(n, ds[0]->n_original, column_base, columns, n, std::move(values));
}

std::ostream &operator<<(std::ostream &os, const Dense &m) {
    std::cout.precision(5);
    int i = 0;
    for (int r = 0; r < m.n_original; r++) {
        for (int c = 0; c < m.columns_total; c++) {
            if (m.column_base <= c && c < m.column_base + m.columns) {
                if (c >= m.n_original) {
                    i++;
                    continue;
                }
                os << m.values[i++];
            } else {
                os << "0.000";
            }
            os << "\t";
        }
        os << std::endl;
    }
    return os;
}

Sparse::Sparse(int n, std::vector<double> &&values, std::vector<int> &&rows_number_of_values,
               std::vector<int> &&values_column) : n{n}, values{values},
                                                    rows_number_of_values{rows_number_of_values},
                                                    values_column{values_column} {}

std::vector<Sparse> Sparse::Split(int processes, bool split_by_column) {
    std::vector<std::vector<double>> m_values(processes);
    std::vector<int> m_last_row(processes);
    std::vector<std::vector<int>> m_rows_values(processes);
    std::vector<std::vector<int>> m_value_column(processes);

    int block_width = block_column_size(n, processes);
    int it = 0;
    for (int row = 0; row < n; row++) {
        int values_in_row = rows_number_of_values[row + 1] - rows_number_of_values[row];
        for (int i = 0; i < values_in_row; i++) {
            int column = values_column[it];
            int part;
            if (split_by_column) {
                part = column / block_width;
            } else {
                part = row / block_width;
            }
            if (m_rows_values[part].empty()) {
                m_rows_values[part].push_back(0);
            }
            if (row > m_last_row[part]) {
                for (int w = 0; w < (row - m_last_row[part]); w++) {
                    m_rows_values[part].push_back(m_values[part].size());
                }
                m_last_row[part] = row;
            }
            m_values[part].push_back(values[it]);
            m_value_column[part].push_back(column);
            it++;
        }
    }

    std::vector<Sparse> matrices;
    for (int i = 0; i < processes; i++) {
        m_rows_values[i].push_back(m_values[i].size());
        auto m = Sparse(n, std::move(m_values[i]), std::move(m_rows_values[i]), std::move(m_value_column[i]));
        matrices.push_back(m);
    }
    return matrices;
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

// sitCmp compares SparseIt iterators.
// It returns True if value of the first one is before the second one.
// Firstly compares X and Y. It also checks if iterator is EOF (value==0).
bool sitCmp(std::tuple<int,int,double> a, std::tuple<int,int,double> b) {
    if (std::get<0>(a) == -1) {
        return false;
    }
    if (std::get<0>(b) == -1 || std::get<0>(a) < std::get<0>(b)) {
        return true;
    } else if (std::get<0>(a) == std::get<0>(b)) {
        return std::get<1>(a) < std::get<1>(b);
    }
    return false;
}

Sparse::Sparse(Sparse *a, Sparse *b) {
    // Initialize iterators over 'a' and 'b'.
    auto ait = SparseIt(a);
    auto bit = SparseIt(b);
    // Initialize values for the new Sparse matrix.
    n = a->n;
    size_t items = a->values.size() + b->values.size();
    assert(items < values.max_size());
    values.resize(items);
    rows_number_of_values.push_back(0);
    assert(items < values_column.max_size());
    values_column.resize(items);
    // Initialize variables for the while loop.
    // Iterator's values, row, column, last_row.
    size_t i = 0;
    ait.Next(); bit.Next();
    auto av = ait.Value();
    auto bv = bit.Value();
    std::tuple<int,int,double> v;
    int last_row = 0;
    int r, c;

    while (i < items) {
        // Determine which value (if matrix 'a' or 'b') should be added first.
        if (sitCmp(av, bv)) {
            v = av;
            ait.Next();
        } else {
            v = bv;
            bit.Next();
        }
        assert(std::get<0>(v) != -1);
        // Add value to the new Matrix.
        r = std::get<0>(v); c = std::get<1>(v);
        values[i] = std::get<double>(v);
        values_column[i] = c;
        if (r > last_row) {
            for (int j = 0; j < (r - last_row); j++)
                rows_number_of_values.push_back(i);
            last_row = r;
        }
        // Update values.
        i++;
        av = ait.Value();
        bv = bit.Value();
    }
    rows_number_of_values.push_back(items);
}

SparseIt::SparseIt(matrix::Sparse *m) : _m{m} {}

std::tuple<int,int,double> SparseIt::Value() {
    if (i >= static_cast<int>(_m->values.size())) {
        return std::make_tuple(-1, -1, 0);
    }
    return std::make_tuple(r, _m->values_column[i], _m->values[i]);
}

bool SparseIt::Next() {
    i++;
    if (i >= static_cast<int>(_m->values.size())) {
        return false;
    }
    if (--_values_in_row <= 0) {
        r++;
        while (_m->rows_number_of_values[r+1] - _m->rows_number_of_values[r] == 0) {
            r++;
        }
        _values_in_row = _m->rows_number_of_values[r+1] - _m->rows_number_of_values[r];
    }
    return true;
}

}
