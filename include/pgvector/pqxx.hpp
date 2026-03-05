/*
 * pgvector-cpp v0.2.4
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <cstddef>
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

        std::vector<float> values;
        if (text.size() > 2) {
            std::string_view inner = text.substr(1, text.size() - 2);
            size_t start = 0;
            for (size_t i = 0; i < inner.size(); i++) {
                if (inner[i] == ',') {
                    values.push_back(string_traits<float>::from_string(inner.substr(start, i - start), c));
                    start = i + 1;
                }
            }
            values.push_back(string_traits<float>::from_string(inner.substr(start), c));
        }
        return pgvector::Vector(std::move(values));
    }

    static std::string_view to_buf(std::span<char> buf, const pgvector::Vector& value, ctx c = {}) {
        std::span<const float> values{value};

        // important! size_buffer cannot throw an exception on overflow
        // so perform this check before writing any data
        if (values.size() > 16000) {
            throw conversion_overrun{"vector cannot have more than 16000 dimensions"};
        }

        size_t here = 0;
        buf[here++] = '[';

        for (size_t i = 0; i < values.size(); i++) {
            if (i != 0) {
                buf[here++] = ',';
            }
            here += pqxx::into_buf(buf.subspan(here), values[i], c);
        }

        buf[here++] = ']';

        return {std::data(buf), here};
    }

    static size_t size_buffer(const pgvector::Vector& value) noexcept {
        std::span<const float> values{value};

        // cannot throw an exception here on overflow
        // so throw in into_buf

        size_t size = 2; // [ and ]
        for (const auto v : values) {
            size += 1; // ,
            size += string_traits<float>::size_buffer(v);
        }
        return size;
    }
};

template <> inline constexpr std::string_view name_type<pgvector::HalfVector>() noexcept { return "halfvec"; };

template <> struct nullness<pgvector::HalfVector> : no_null<pgvector::HalfVector> {};

template <> struct string_traits<pgvector::HalfVector> {
    static pgvector::HalfVector from_string(std::string_view text, ctx c = {}) {
        if (text.size() < 2 || text.front() != '[' || text.back() != ']') {
            throw conversion_error("Malformed halfvec literal");
        }

        std::vector<pgvector::Half> values;
        if (text.size() > 2) {
            std::string_view inner = text.substr(1, text.size() - 2);
            size_t start = 0;
            for (size_t i = 0; i < inner.size(); i++) {
                if (inner[i] == ',') {
                    values.push_back(static_cast<pgvector::Half>(string_traits<float>::from_string(inner.substr(start, i - start), c)));
                    start = i + 1;
                }
            }
            values.push_back(static_cast<pgvector::Half>(string_traits<float>::from_string(inner.substr(start), c)));
        }
        return pgvector::HalfVector(std::move(values));
    }

    static std::string_view to_buf(std::span<char> buf, const pgvector::HalfVector& value, ctx c = {}) {
        std::span<const pgvector::Half> values{value};

        // important! size_buffer cannot throw an exception on overflow
        // so perform this check before writing any data
        if (values.size() > 16000) {
            throw conversion_overrun{"halfvec cannot have more than 16000 dimensions"};
        }

        size_t here = 0;
        buf[here++] = '[';

        for (size_t i = 0; i < values.size(); i++) {
            if (i != 0) {
                buf[here++] = ',';
            }
            here += pqxx::into_buf(buf.subspan(here), static_cast<float>(values[i]), c);
        }

        buf[here++] = ']';

        return {std::data(buf), here};
    }

    static size_t size_buffer(const pgvector::HalfVector& value) noexcept {
        std::span<const pgvector::Half> values{value};

        // cannot throw an exception here on overflow
        // so throw in into_buf

        size_t size = 2; // [ and ]
        for (const auto v : values) {
            size += 1; // ,
            size += string_traits<float>::size_buffer(static_cast<float>(v));
        }
        return size;
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
            auto add_element = [&](std::string_view substr) {
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
            };

            std::string_view inner = text.substr(1, n - 1);
            size_t start = 0;
            for (size_t i = 0; i < inner.size(); i++) {
                if (inner[i] == ',') {
                    add_element(inner.substr(start, i - start));
                    start = i + 1;
                }
            }
            add_element(inner.substr(start));
        }

        return pgvector::SparseVector(dimensions, std::move(indices), std::move(values));
    }

    static std::string_view to_buf(std::span<char> buf, const pgvector::SparseVector& value, ctx c = {}) {
        int dimensions = value.dimensions();
        auto& indices = value.indices();
        auto& values = value.values();
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
        auto& indices = value.indices();
        auto& values = value.values();
        size_t nnz = indices.size();

        // cannot throw an exception here on overflow
        // so throw in into_buf

        size_t size = 3; // {, }, and /
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
