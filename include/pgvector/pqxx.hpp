/*
 * pgvector-cpp v0.2.4
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <pqxx/pqxx>

#include "halfvec.hpp"
#include "sparsevec.hpp"
#include "vector.hpp"

/// @cond

namespace pqxx {
template <> inline constexpr std::string_view name_type<pgvector::Vector>() noexcept { return "vector"; };

template <> struct nullness<pgvector::Vector> : no_null<pgvector::Vector> {};

template <> struct string_traits<pgvector::Vector> {
    static pgvector::Vector from_string(std::string_view text, ctx c = {}) {
        if (text.size() < 2 || text.front() != '[' || text.back() != ']') {
            throw conversion_error("Malformed vector literal");
        }

        std::vector<float> result;
        if (text.size() > 2) {
            // TODO don't copy string
            std::istringstream ss(std::string(text.substr(1, text.size() - 2)));
            while (ss.good()) {
                std::string substr;
                std::getline(ss, substr, ',');
                result.push_back(string_traits<float>::from_string(substr, c));
            }
        }
        return pgvector::Vector(result);
    }

    static std::string_view to_buf(std::span<char> buf, const pgvector::Vector& value, ctx c = {}) {
        auto len = pqxx::into_buf(buf, static_cast<std::vector<float>>(value), c);
        // replace array brackets
        buf[0] = '[';
        buf[len - 1] = ']';
        return {std::data(buf), len};
    }

    static size_t size_buffer(const pgvector::Vector& value) noexcept {
        return string_traits<std::vector<float>>::size_buffer(
            static_cast<std::vector<float>>(value));
    }
};

template <> inline constexpr std::string_view name_type<pgvector::HalfVector>() noexcept { return "halfvec"; };

template <> struct nullness<pgvector::HalfVector> : no_null<pgvector::HalfVector> {};

template <> struct string_traits<pgvector::HalfVector> {
    static pgvector::HalfVector from_string(std::string_view text, ctx c = {}) {
        if (text.size() < 2 || text.front() != '[' || text.back() != ']') {
            throw conversion_error("Malformed halfvec literal");
        }

        std::vector<float> result;
        if (text.size() > 2) {
            // TODO don't copy string
            std::istringstream ss(std::string(text.substr(1, text.size() - 2)));
            while (ss.good()) {
                std::string substr;
                std::getline(ss, substr, ',');
                result.push_back(string_traits<float>::from_string(substr, c));
            }
        }
        return pgvector::HalfVector(result);
    }

    static std::string_view to_buf(std::span<char> buf, const pgvector::HalfVector& value, ctx c = {}) {
        auto len = pqxx::into_buf(buf, static_cast<std::vector<float>>(value), c);
        // replace array brackets
        buf[0] = '[';
        buf[len - 1] = ']';
        return {std::data(buf), len};
    }

    static size_t size_buffer(const pgvector::HalfVector& value) noexcept {
        return string_traits<std::vector<float>>::size_buffer(
            static_cast<std::vector<float>>(value));
    }
};

template <> inline constexpr std::string_view name_type<pgvector::SparseVector>() noexcept { return "sparsevec"; };

template <> struct nullness<pgvector::SparseVector> : no_null<pgvector::SparseVector> {};

template <> struct string_traits<pgvector::SparseVector> {
    static pgvector::SparseVector from_string(std::string_view text, ctx c = {}) {
        if (text.size() < 4 || text.front() != '{') {
            throw conversion_error("Malformed sparsevec literal");
        }

        size_t n = text.find("}/", 1);
        if (n == std::string_view::npos) {
            throw conversion_error("Malformed sparsevec literal");
        }

        int dimensions = string_traits<int>::from_string(text.substr(n + 2), c);
        if (dimensions < 0) {
            throw conversion_error("Dimensions cannot be negative");
        }

        std::vector<int> indices;
        std::vector<float> values;

        if (n > 1) {
            std::istringstream ss(std::string(text.substr(1, n - 1)));
            while (ss.good()) {
                std::string substr;
                std::getline(ss, substr, ',');

                size_t ne = substr.find(":");
                if (ne == std::string::npos) {
                    throw conversion_error("Malformed sparsevec literal");
                }

                int index = string_traits<int>::from_string(substr.substr(0, ne), c);
                float value = string_traits<float>::from_string(substr.substr(ne + 1), c);

                if (index < 1) {
                    throw conversion_error("Index out of bounds");
                }

                indices.push_back(index - 1);
                values.push_back(value);
            }
        }

        return pgvector::SparseVector(dimensions, indices, values);
    }

    static std::string_view to_buf(std::span<char> buf, const pgvector::SparseVector& value, ctx c = {}) {
        int dimensions = value.dimensions();
        auto indices = value.indices();
        auto values = value.values();
        size_t nnz = indices.size();

        // important! size_buffer cannot throw an exception on overflow
        // so perform this check before writing any data
        if (nnz > 16000) {
            throw conversion_overrun{"sparsevec cannot have more than 16000 dimensions"};
        }

        size_t here = 0;
        buf[here++] = '{';

        for (size_t i = 0; i < nnz; i++) {
            if (i != 0) {
                buf[here++] = ',';
            }

            here += pqxx::into_buf(buf.subspan(here), indices[i] + 1, c);
            buf[here++] = ':';
            here += pqxx::into_buf(buf.subspan(here), values[i], c);
        }

        buf[here++] = '}';
        buf[here++] = '/';
        here += pqxx::into_buf(buf.subspan(here), dimensions, c);

        return {std::data(buf), here};
    }

    static size_t size_buffer(const pgvector::SparseVector& value) noexcept {
        int dimensions = value.dimensions();
        auto indices = value.indices();
        auto values = value.values();
        size_t nnz = indices.size();

        // cannot throw an exception here on overflow
        // so throw in into_buf

        size_t size = 4; // {, }, /, and \0
        size += string_traits<int>::size_buffer(dimensions);
        for (size_t i = 0; i < nnz; i++) {
            size += 2; // : and ,
            size += string_traits<int>::size_buffer(indices[i]);
            size += string_traits<float>::size_buffer(values[i]);
        }
        return size;
    }
};
} // namespace pqxx

/// @endcond
