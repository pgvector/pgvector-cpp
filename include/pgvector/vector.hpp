/*
 * pgvector-cpp v0.3.0
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <cstddef>
#include <ostream>
#include <span>
#include <utility>
#include <vector>

namespace pgvector {
/// A vector.
class Vector {
  public:
    /// Creates a vector from a `std::vector`.
    explicit Vector(const std::vector<float>& value) : value_{value} {}

    /// Creates a vector from a `std::vector`.
    explicit Vector(std::vector<float>&& value) : value_{std::move(value)} {}

    /// Creates a vector from a span.
    explicit Vector(std::span<const float> value) : value_{std::vector<float>(value.begin(), value.end())} {}

    /// Returns the number of dimensions.
    size_t dimensions() const {
        return value_.size();
    }

    /// Returns the values.
    const std::vector<float>& values() const {
        return value_;
    }

    friend bool operator==(const Vector& lhs, const Vector& rhs) {
        return lhs.value_ == rhs.value_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Vector& value) {
        os << "[";
        // TODO use std::views::enumerate for C++23
        size_t i = 0;
        for (auto v : value.value_) {
            if (i > 0) {
                os << ",";
            }
            os << v;
            i++;
        }
        os << "]";
        return os;
    }

  private:
    std::vector<float> value_;
};
} // namespace pgvector
