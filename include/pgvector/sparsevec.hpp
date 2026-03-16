/*
 * pgvector-cpp v0.3.0
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <ostream>
#include <span>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace pgvector {
/// A sparse vector.
class SparseVector {
  public:
    /// Creates a sparse vector from a dense vector.
    explicit SparseVector(const std::vector<float>& value) :
        SparseVector(std::span<const float>{value}) {}

    /// Creates a sparse vector from a span.
    explicit SparseVector(std::span<const float> value) {
        if (value.size() > std::numeric_limits<int>::max()) {
            throw std::invalid_argument{"sparsevec cannot have more than max int dimensions"};
        }
        dimensions_ = static_cast<int>(value.size());

        // do not reserve capacity for indices/values since likely many zeros
        // TODO use std::views::enumerate for C++23
        size_t i = 0;
        for (auto v : value) {
            if (v != 0) {
                indices_.push_back(static_cast<int>(i));
                values_.push_back(v);
            }
            i++;
        }
    }

    /// Creates a sparse vector from a map of non-zero elements.
    SparseVector(const std::unordered_map<int, float>& map, int dimensions) {
        if (dimensions < 0) {
            throw std::invalid_argument{"sparsevec cannot have negative dimensions"};
        }
        dimensions_ = dimensions;

        // could probably reserve capacity for indices since not expecting zeros
        for (const auto& [i, v] : map) {
            if (i < 0 || i >= dimensions) {
                throw std::invalid_argument{"sparsevec index out of bounds"};
            }

            if (v != 0) {
                indices_.push_back(i);
            }
        }
        std::ranges::sort(indices_);

        values_.reserve(indices_.size());
        for (const auto i : indices_) {
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
        return lhs.dimensions_ == rhs.dimensions_ && lhs.indices_ == rhs.indices_
            && lhs.values_ == rhs.values_;
    }

    friend std::ostream& operator<<(std::ostream& os, const SparseVector& value) {
        os << "{";
        // TODO use std::views::zip for C++23
        for (size_t i = 0; i < value.indices_.size(); i++) {
            if (i > 0) {
                os << ",";
            }
            os << value.indices_.at(i) + 1;
            os << ":";
            os << value.values_.at(i);
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
