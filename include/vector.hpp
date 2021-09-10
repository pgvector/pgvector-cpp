#include <ostream>
#include <vector>

namespace pgvector {
class Vector {
public:
  Vector(const std::vector<float> &value) {
    value_ = value;
  }

  Vector(const float *value, size_t n) {
    value_ = std::vector<float>{value, value + n};
  }

  operator const std::vector<float>() const {
    return value_;
  }

  friend bool operator==(const Vector &lhs, const Vector &rhs) {
    return lhs.value_ == rhs.value_;
  }

  friend std::ostream &operator<<(std::ostream &os, const Vector &value) {
    os << "[";
    for (auto i = 0; i < value.value_.size(); i++) {
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
