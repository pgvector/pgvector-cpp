/*!
 * pgvector-cpp v0.2.2
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <algorithm>
#include <ostream>
#include <unordered_map>
#include <vector>

#if __cplusplus >= 202002L
#include <span>
#endif

namespace pgvector {
/// A sparse vector.
class SparseVector {
public:
    /// @private
    // TODO remove in 0.3.0
    SparseVector() = default;

    /// @private
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

#if __cplusplus >= 202002L
    /// Creates a sparse vector from a span.
    SparseVector(std::span<const float> value) {
        dimensions_ = value.size();
        for (size_t i = 0; i < value.size(); i++) {
            float v = value[i];
            if (v != 0) {
                indices_.push_back(i);
                values_.push_back(v);
            }
        }
    }
#endif

    /// Creates a sparse vector from a map of non-zero elements.
    SparseVector(const std::unordered_map<int, float>& map, int dimensions) {
        if (dimensions < 1) {
            throw std::invalid_argument("sparsevec must have at least 1 dimension");
        }
        dimensions_ = dimensions;

        for (auto [i, v] : map) {
            if (i < 0 || i >= dimensions) {
                throw std::invalid_argument("sparsevec index out of bounds");
            }

            if (v != 0) {
                indices_.push_back(i);
            }
        }
        std::sort(indices_.begin(), indices_.end());

        values_.reserve(indices_.size());
        for (auto i : indices_) {
            values_.push_back(map.at(i));
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
