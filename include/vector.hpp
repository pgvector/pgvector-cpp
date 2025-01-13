/*!
 * pgvector-cpp v0.2.0
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <ostream>
#include <vector>

#if __cplusplus >= 202002L
#include <span>
#endif

namespace pgvector {
class Vector {
public:
  Vector() = default;

  Vector(const std::vector<float>& value) {
    value_ = value;
  }

  Vector(std::vector<float>&& value) {
    value_ = std::move(value);
  }

  Vector(const float* value, size_t n) {
    value_ = std::vector<float>{value, value + n};
  }

#if __cplusplus >= 202002L
  Vector(std::span<const float> value) {
    value_ = std::vector<float>(value.begin(), value.end());
  }
#endif

  size_t dimensions() const {
    return value_.size();
  }

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
