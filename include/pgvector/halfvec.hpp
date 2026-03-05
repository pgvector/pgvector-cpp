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

#if __STDCPP_FLOAT16_T__
#include <stdfloat>
#endif

namespace pgvector {

#if __STDCPP_FLOAT16_T__
using HalfType = std::float16_t;
#else
using HalfType = float;
#endif

/// A half vector.
class HalfVector {
  public:
    /// Creates a half vector from a `std::vector<pgvector::HalfType>`.
    explicit HalfVector(const std::vector<HalfType>& value) : value_{value} {}

    /// Creates a half vector from a `std::vector<pgvector::HalfType>`.
    explicit HalfVector(std::vector<HalfType>&& value) : value_{std::move(value)} {}

    /// Creates a half vector from a span.
    explicit HalfVector(std::span<const HalfType> value) : value_{std::vector<HalfType>(value.begin(), value.end())} {}

    /// Returns the number of dimensions.
    size_t dimensions() const {
        return value_.size();
    }

    /// Returns the half vector as a `std::vector<pgvector::HalfType>`.
    operator const std::vector<HalfType>() const {
        return value_;
    }

    /// Returns the half vector as a `std::span<const pgvector::HalfType>`.
    operator const std::span<const HalfType>() const {
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
    std::vector<HalfType> value_;
};
} // namespace pgvector
