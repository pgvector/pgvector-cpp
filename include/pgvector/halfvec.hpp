/*
 * pgvector-cpp v0.2.4
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
/// A half vector.
class HalfVector {
  public:
    /// Creates a half vector from a `std::vector<float>`.
    explicit HalfVector(const std::vector<float>& value) {
        value_ = value;
    }

    /// Creates a half vector from a `std::vector<float>`.
    explicit HalfVector(std::vector<float>&& value) {
        value_ = std::move(value);
    }

    /// Creates a half vector from a span.
    explicit HalfVector(std::span<const float> value) {
        value_ = std::vector<float>(value.begin(), value.end());
    }

    /// Returns the number of dimensions.
    size_t dimensions() const {
        return value_.size();
    }

    /// Returns the half vector as a `std::vector<float>`.
    operator const std::vector<float>() const {
        return value_;
    }

    friend bool operator==(const HalfVector& lhs, const HalfVector& rhs) {
        return lhs.value_ == rhs.value_;
    }

    friend std::ostream& operator<<(std::ostream& os, const HalfVector& value) {
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
    // TODO use std::float16_t for C++23
    std::vector<float> value_;
};
} // namespace pgvector
