/*!
 * pgvector-cpp v0.2.1
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <ostream>
#include <vector>

namespace pgvector {
/// A sparse vector.
class SparseVector {
public:
    // TODO remove in 0.3.0
    SparseVector() = default;

    SparseVector(int dimensions, const std::vector<int>& indices, const std::vector<float>& values) {
        if (values.size() != indices.size()) {
            throw std::invalid_argument("indices and values must be the same length");
        }
        dimensions_ = dimensions;
        indices_ = indices;
        values_ = values;
    }

    /// Creates a sparse vector from a dense vector.
    SparseVector(const std::vector<float>& value) {
        dimensions_ = value.size();
        for (size_t i = 0; i < value.size(); i++) {
            float v = value[i];
            if (v != 0) {
                indices_.push_back(i);
                values_.push_back(v);
            }
        }
    }

    /// Returns the number of dimensions.
    int dimensions() const {
        return dimensions_;
    }

    /// Returns the non-zero indices.
    const std::vector<int>& indices() const {
        return indices_;
    }

    /// Returns the non-zero values.
    const std::vector<float>& values() const {
        return values_;
    }

    friend bool operator==(const SparseVector& lhs, const SparseVector& rhs) {
        return lhs.dimensions_ == rhs.dimensions_ && lhs.indices_ == rhs.indices_ && lhs.values_ == rhs.values_;
    }

    friend std::ostream& operator<<(std::ostream& os, const SparseVector& value) {
        os << "{";
        for (size_t i = 0; i < value.indices_.size(); i++) {
            if (i > 0) {
                os << ",";
            }
            os << value.indices_[i] + 1;
            os << ":";
            os << value.values_[i];
        }
        os << "}/";
        os << value.dimensions_;
        return os;
    }

private:
    int dimensions_;
    std::vector<int> indices_;
    std::vector<float> values_;
};
} // namespace pgvector
