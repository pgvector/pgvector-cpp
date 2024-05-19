/*!
 * pgvector-cpp v0.1.1
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <ostream>
#include <vector>

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
