/*!
 * pgvector-cpp v0.2.2
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include "halfvec.hpp"
#include "sparsevec.hpp"
#include "vector.hpp"
#include <pqxx/pqxx>
#include <sstream>

/// @cond

namespace pqxx {
template <> std::string const type_name<pgvector::Vector>{"vector"};

template <> struct nullness<pgvector::Vector> : pqxx::no_null<pgvector::Vector> {};

template <> struct string_traits<pgvector::Vector> {
    static constexpr bool converts_to_string{true};

    static constexpr bool converts_from_string{true};

    static pgvector::Vector from_string(std::string_view text) {
        if (text.front() != '[' || text.back() != ']') {
            throw conversion_error("Malformed vector literal");
        }

        // TODO don't copy string
        std::vector<float> result;
        std::stringstream ss(std::string(text.substr(1, text.size() - 2)));
        while (ss.good()) {
            std::string substr;
            getline(ss, substr, ',');
            result.push_back(std::stof(substr));
        }
        return pgvector::Vector(result);
    }

    static zview to_buf(char* begin, char* end, const pgvector::Vector& value) {
        char *const next = into_buf(begin, end, value);
        return zview{begin, next - begin - 1};
    }

    static char* into_buf(char* begin, char* end, const pgvector::Vector& value) {
        auto ret = string_traits<std::vector<float>>::into_buf(
            begin, end, static_cast<std::vector<float>>(value));
        // replace array brackets
        *begin = '[';
        *(ret - 2) = ']';
        return ret;
    }

    static size_t size_buffer(const pgvector::Vector& value) noexcept {
        return string_traits<std::vector<float>>::size_buffer(
            static_cast<std::vector<float>>(value));
    }
};

template <> std::string const type_name<pgvector::HalfVector>{"halfvec"};

template <> struct nullness<pgvector::HalfVector> : pqxx::no_null<pgvector::HalfVector> {};

template <> struct string_traits<pgvector::HalfVector> {
    static constexpr bool converts_to_string{true};

    static constexpr bool converts_from_string{true};

    static pgvector::HalfVector from_string(std::string_view text) {
        if (text.front() != '[' || text.back() != ']') {
            throw conversion_error("Malformed halfvec literal");
        }

        // TODO don't copy string
        std::vector<float> result;
        std::stringstream ss(std::string(text.substr(1, text.size() - 2)));
        while (ss.good()) {
            std::string substr;
            getline(ss, substr, ',');
            result.push_back(std::stof(substr));
        }
        return pgvector::HalfVector(result);
    }

    static zview to_buf(char* begin, char* end, const pgvector::HalfVector& value) {
        char *const next = into_buf(begin, end, value);
        return zview{begin, next - begin - 1};
    }

    static char* into_buf(char* begin, char* end, const pgvector::HalfVector& value) {
        auto ret = string_traits<std::vector<float>>::into_buf(
            begin, end, static_cast<std::vector<float>>(value));
        // replace array brackets
        *begin = '[';
        *(ret - 2) = ']';
        return ret;
    }

    static size_t size_buffer(const pgvector::HalfVector& value) noexcept {
        return string_traits<std::vector<float>>::size_buffer(
            static_cast<std::vector<float>>(value));
    }
};

template <> std::string const type_name<pgvector::SparseVector>{"sparsevec"};

template <> struct nullness<pgvector::SparseVector> : pqxx::no_null<pgvector::SparseVector> {};

template <> struct string_traits<pgvector::SparseVector> {
    static constexpr bool converts_to_string{true};

    // TODO add from_string
    static constexpr bool converts_from_string{false};

    static zview to_buf(char* begin, char* end, const pgvector::SparseVector& value) {
        char *const next = into_buf(begin, end, value);
        return zview{begin, next - begin - 1};
    }

    static char* into_buf(char* begin, char* end, const pgvector::SparseVector& value) {
        int dimensions = value.dimensions();
        auto indices = value.indices();
        auto values = value.values();
        size_t nnz = indices.size();

        // important! size_buffer cannot throw an exception on overflow
        // so perform this check before writing any data
        if (nnz > 16000) {
            throw conversion_overrun{"sparsevec cannot have more than 16000 dimensions"};
        }

        char *here = begin;
        *here++ = '{';

        for (size_t i = 0; i < nnz; i++) {
            if (i != 0) {
                *here++ = ',';
            }

            here = string_traits<int>::into_buf(here, end, indices[i] + 1) - 1;
            *here++ = ':';
            here = string_traits<float>::into_buf(here, end, values[i]) - 1;
        }

        *here++ = '}';
        *here++ = '/';
        here = string_traits<int>::into_buf(here, end, dimensions) - 1;
        *here++ = '\0';

        return here;
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
