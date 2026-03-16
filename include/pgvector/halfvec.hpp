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

#if __STDCPP_FLOAT16_T__
#include <stdfloat>
#else
#define __STDC_WANT_IEC_60559_TYPES_EXT__
#include <cfloat>
#endif

namespace pgvector {
/// A half vector element.
#if __STDCPP_FLOAT16_T__
using Half = std::float16_t;
#elif defined(__FLT16_MAX__)
using Half = _Float16;
#else
using Half = float;
#endif

/// A half vector.
class HalfVector {
  public:
    /// Creates a half vector from a `std::vector`.
    explicit HalfVector(const std::vector<Half>& value) : value_{value} {}

    /// Creates a half vector from a `std::vector`.
    explicit HalfVector(std::vector<Half>&& value) : value_{std::move(value)} {}

    /// Creates a half vector from a span.
    explicit HalfVector(std::span<const Half> value) :
        value_{std::vector<Half>(value.begin(), value.end())} {}

    /// Returns the number of dimensions.
    size_t dimensions() const {
        return value_.size();
    }

    /// Returns the values.
    const std::vector<Half>& values() const {
        return value_;
    }

    friend bool operator==(const HalfVector& lhs, const HalfVector& rhs) {
        return lhs.value_ == rhs.value_;
    }

    friend std::ostream& operator<<(std::ostream& os, const HalfVector& value) {
        os << "[";
        // TODO use std::views::enumerate for C++23
        size_t i = 0;
        for (auto v : value.value_) {
            if (i > 0) {
                os << ",";
            }
#if __STDCPP_FLOAT16_T__
            os << v;
#else
            os << static_cast<float>(v);
#endif
            i++;
        }
        os << "]";
        return os;
    }

  private:
    std::vector<Half> value_;
};
} // namespace pgvector
