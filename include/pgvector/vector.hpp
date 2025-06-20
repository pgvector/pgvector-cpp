/*
 * pgvector-cpp v0.2.2
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>

#if __cplusplus >= 202002L
#include <span>
#endif

namespace pgvector {
/// A vector.
class Vector {
  public:
    /// @private
    // TODO remove in 0.3.0
    Vector() = default;

    /// Creates a vector from a `std::vector<float>`.
    // TODO add explicit in 0.3.0
    Vector(const std::vector<float>& value) {
        value_ = value;
    }

    /// Creates a vector from a `std::vector<float>`.
    // TODO add explicit in 0.3.0
    Vector(std::vector<float>&& value) {
        value_ = std::move(value);
    }

    /// Creates a vector from an array.
    Vector(const float* value, size_t n) {
        value_ = std::vector<float>{value, value + n};
    }

#if __cplusplus >= 202002L
    /// Creates a vector from a span.
    // TODO add explicit in 0.3.0
    Vector(std::span<const float> value) {
        value_ = std::vector<float>(value.begin(), value.end());
    }
#endif

    /// Returns the number of dimensions.
    size_t dimensions() const {
        return value_.size();
    }

    /// Returns the vector as a `std::vector<float>`.
    operator const std::vector<float>() const {
        return value_;
    }

    friend bool operator==(const Vector& lhs, const Vector& rhs) {
        return lhs.value_ == rhs.value_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Vector& value) {
        os << "[";
        for (size_t i = 0; i < value.value_.size(); i++) {
            if (i > 0) {
                os << ",";
            }
            os << value.value_[i];
        }
        os << "]";
        return os;
    }

  private:
    std::vector<float> value_;
};
} // namespace pgvector
