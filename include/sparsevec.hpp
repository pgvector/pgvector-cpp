/*!
 * pgvector-cpp v0.1.1
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <ostream>
#include <vector>

namespace pgvector {
class SparseVector {
public:
  SparseVector() = default;

  SparseVector(int dimensions, const std::vector<int>& indices, const std::vector<float>& values) {
    if (values.size() != indices.size()) {
      throw std::invalid_argument("indices and values must be the same length");
    }
    dimensions_ = dimensions;
    indices_ = indices;
    values_ = values;
  }

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

  int dimensions() const {
    return dimensions_;
  }

  const std::vector<int>& indices() const {
    return indices_;
  }

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
