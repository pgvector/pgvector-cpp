/*!
 * pgvector-cpp v0.1.0
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include "vector.hpp"
#include <pqxx/pqxx>
#include <sstream>

namespace pqxx {
template <> std::string const type_name<pgvector::Vector>{"vector"};

template <> struct nullness<pgvector::Vector> : pqxx::no_null<pgvector::Vector> {};

template <> struct string_traits<pgvector::Vector> {
  static pgvector::Vector from_string(std::string_view text) {
    if (text.front() != '[' || text.back() != ']') {
      throw conversion_error("Malformed vector literal");
    }

    // TODO don't copy string
    std::vector<float> result;
    std::stringstream ss(std::string(text.substr(1, -2)));
    while (ss.good()) {
      std::string substr;
      getline(ss, substr, ',');
      result.push_back(std::stod(substr));
    }
    return pgvector::Vector(result);
  }

  static zview to_buf(char *begin, char *end, pgvector::Vector const &value) {
    char *const next = into_buf(begin, end, value);
    return zview{begin, next - begin - 1};
  }

  static char *into_buf(char *begin, char *end, pgvector::Vector const &value) {
    auto ret = string_traits<std::vector<float>>::into_buf(
        begin, end, static_cast<std::vector<float>>(value));
    // replace array brackets
    *begin = '[';
    *(ret - 2) = ']';
    return ret;
  }

  static size_t size_buffer(pgvector::Vector const &value) noexcept {
    return string_traits<std::vector<float>>::size_buffer(
        static_cast<std::vector<float>>(value));
  }
};
} // namespace pqxx
