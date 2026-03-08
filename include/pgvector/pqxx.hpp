/*
 * pgvector-cpp v0.3.0
 * https://github.com/pgvector/pgvector-cpp
 * MIT License
 */

#pragma once

#include <cstddef>
#include <limits>
#include <ranges>
#include <string_view>
#include <vector>

#include <pqxx/pqxx>

#include "halfvec.hpp"
#include "sparsevec.hpp"
#include "vector.hpp"

/// @cond

namespace pqxx {
template<> inline constexpr std::string_view name_type<pgvector::Vector>() noexcept {
    return "vector";
};

template<> struct nullness<pgvector::Vector> : no_null<pgvector::Vector> {};

template<> struct string_traits<pgvector::Vector> {
    static pgvector::Vector from_string(std::string_view text, ctx c = {}) {
        if (text.size() < 2 || text.front() != '[' || text.back() != ']') {
            throw conversion_error{"Malformed vector literal"};
        }

        std::vector<float> values;
        if (text.size() > 2) {
            std::string_view inner = text.substr(1, text.size() - 2);
            for (const auto& v : std::views::split(inner, ',')) {
                std::string_view sv{v.begin(), v.end()};
                values.push_back(pqxx::from_string<float>(sv, c));
            }
        }
        return pgvector::Vector{std::move(values)};
    }

    static std::string_view to_buf(std::span<char> buf, const pgvector::Vector& value, ctx c = {}) {
        // confirm caller provided estimated buffer space
        if (buf.size() < size_buffer(value)) {
            throw conversion_overrun{"Not enough space in buffer for vector"};
        }

        const std::vector<float>& values = value.values();

        // important! size_buffer cannot throw an exception on overflow
        // so perform this check before writing any data
        if (values.size() > 16000) {
            throw conversion_overrun{"vector cannot have more than 16000 dimensions"};
        }

        size_t here = 0;
        here += pqxx::into_buf(buf.subspan(here), "[", c);

        for (size_t i = 0; i < values.size(); i++) {
            if (i != 0) {
                here += pqxx::into_buf(buf.subspan(here), ",", c);
            }
            here += pqxx::into_buf(buf.subspan(here), values[i], c);
        }

        here += pqxx::into_buf(buf.subspan(here), "]", c);

        return {std::data(buf), here};
    }

    static size_t size_buffer(const pgvector::Vector& value) noexcept {
        const std::vector<float>& values = value.values();

        // cannot throw an exception here on overflow
        // so throw in into_buf

        size_t size = 0;
        size += pqxx::size_buffer("[");
        for (const auto v : values) {
            size += pqxx::size_buffer(",");
            size += pqxx::size_buffer(v);
        }
        size += pqxx::size_buffer("]");
        return size;
    }
};

template<> inline constexpr std::string_view name_type<pgvector::HalfVector>() noexcept {
    return "halfvec";
};

template<> struct nullness<pgvector::HalfVector> : no_null<pgvector::HalfVector> {};

template<> struct string_traits<pgvector::HalfVector> {
    static pgvector::HalfVector from_string(std::string_view text, ctx c = {}) {
        if (text.size() < 2 || text.front() != '[' || text.back() != ']') {
            throw conversion_error{"Malformed halfvec literal"};
        }

        std::vector<pgvector::Half> values;
        if (text.size() > 2) {
            std::string_view inner = text.substr(1, text.size() - 2);
            for (const auto& v : std::views::split(inner, ',')) {
                std::string_view sv{v.begin(), v.end()};
                values.push_back(static_cast<pgvector::Half>(pqxx::from_string<float>(sv, c)));
            }
        }
        return pgvector::HalfVector{std::move(values)};
    }

    static std::string_view to_buf(std::span<char> buf, const pgvector::HalfVector& value, ctx c = {}) {
        // confirm caller provided estimated buffer space
        if (buf.size() < size_buffer(value)) {
            throw conversion_overrun{"Not enough space in buffer for halfvec"};
        }

        const std::vector<pgvector::Half>& values = value.values();

        // important! size_buffer cannot throw an exception on overflow
        // so perform this check before writing any data
        if (values.size() > 16000) {
            throw conversion_overrun{"halfvec cannot have more than 16000 dimensions"};
        }

        size_t here = 0;
        here += pqxx::into_buf(buf.subspan(here), "[", c);

        for (size_t i = 0; i < values.size(); i++) {
            if (i != 0) {
                here += pqxx::into_buf(buf.subspan(here), ",", c);
            }
            here += pqxx::into_buf(buf.subspan(here), static_cast<float>(values[i]), c);
        }

        here += pqxx::into_buf(buf.subspan(here), "]", c);

        return {std::data(buf), here};
    }

    static size_t size_buffer(const pgvector::HalfVector& value) noexcept {
        const std::vector<pgvector::Half>& values = value.values();

        // cannot throw an exception here on overflow
        // so throw in into_buf

        size_t size = 0;
        size += pqxx::size_buffer("[");
        for (const auto v : values) {
            size += pqxx::size_buffer(",");
            size += pqxx::size_buffer(static_cast<float>(v));
        }
        size += pqxx::size_buffer("]");
        return size;
    }
};

template<> inline constexpr std::string_view name_type<pgvector::SparseVector>() noexcept {
    return "sparsevec";
};

template<> struct nullness<pgvector::SparseVector> : no_null<pgvector::SparseVector> {};

template<> struct string_traits<pgvector::SparseVector> {
    static pgvector::SparseVector from_string(std::string_view text, ctx c = {}) {
        if (text.size() < 4 || text.front() != '{') {
            throw conversion_error{"Malformed sparsevec literal"};
        }

        size_t n = text.find("}/", 1);
        if (n == std::string_view::npos) {
            throw conversion_error{"Malformed sparsevec literal"};
        }

        int dimensions = pqxx::from_string<int>(text.substr(n + 2), c);

        std::unordered_map<int, float> map;
        if (n > 1) {
            std::string_view inner = text.substr(1, n - 1);
            for (const auto& v : std::views::split(inner, ',')) {
                std::string_view sv{v.begin(), v.end()};

                size_t ne = sv.find(":");
                if (ne == std::string::npos) {
                    throw conversion_error{"Malformed sparsevec literal"};
                }

                int index = pqxx::from_string<int>(sv.substr(0, ne), c);
                float value = pqxx::from_string<float>(sv.substr(ne + 1), c);

                // check to avoid undefined behavior
                if (index > std::numeric_limits<int>::min()) {
                    index -= 1;
                }

                map.insert({index, value});
            }
        }

        try {
            return pgvector::SparseVector{map, dimensions};
        } catch (const std::invalid_argument& e) {
            throw conversion_error{e.what()};
        }
    }

    static std::string_view to_buf(std::span<char> buf, const pgvector::SparseVector& value, ctx c = {}) {
        // confirm caller provided estimated buffer space
        if (buf.size() < size_buffer(value)) {
            throw conversion_overrun{"Not enough space in buffer for sparsevec"};
        }

        int dimensions = value.dimensions();
        const std::vector<int>& indices = value.indices();
        const std::vector<float>& values = value.values();
        size_t nnz = indices.size();

        // important! size_buffer cannot throw an exception on overflow
        // so perform this check before writing any data
        if (nnz > 16000) {
            throw conversion_overrun{"sparsevec cannot have more than 16000 dimensions"};
        }

        size_t here = 0;
        here += pqxx::into_buf(buf.subspan(here), "{", c);

        for (size_t i = 0; i < nnz; i++) {
            if (i != 0) {
                here += pqxx::into_buf(buf.subspan(here), ",", c);
            }
            // cast to avoid undefined behavior and require less buffer space
            here += pqxx::into_buf(buf.subspan(here), static_cast<unsigned int>(indices[i]) + 1, c);
            here += pqxx::into_buf(buf.subspan(here), ":", c);
            here += pqxx::into_buf(buf.subspan(here), values[i], c);
        }

        here += pqxx::into_buf(buf.subspan(here), "}/", c);
        here += pqxx::into_buf(buf.subspan(here), dimensions, c);

        return {std::data(buf), here};
    }

    static size_t size_buffer(const pgvector::SparseVector& value) noexcept {
        int dimensions = value.dimensions();
        const std::vector<int>& indices = value.indices();
        const std::vector<float>& values = value.values();
        size_t nnz = indices.size();

        // cannot throw an exception here on overflow
        // so throw in into_buf

        size_t size = 0;
        size += pqxx::size_buffer("{");
        for (size_t i = 0; i < nnz; i++) {
            size += pqxx::size_buffer(",");
            size += pqxx::size_buffer(static_cast<unsigned int>(indices[i]) + 1);
            size += pqxx::size_buffer(":");
            size += pqxx::size_buffer(values[i]);
        }
        size += pqxx::size_buffer("}/");
        size += pqxx::size_buffer(dimensions);
        return size;
    }
};
} // namespace pqxx

/// @endcond
