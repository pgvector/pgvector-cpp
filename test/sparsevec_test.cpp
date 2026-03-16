#include <span>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <pgvector/sparsevec.hpp>

#include "helper.hpp"

using pgvector::SparseVector;

static void test_constructor_vector() {
    SparseVector vec{std::vector<float>{1, 0, 2, 0, 3, 0}};
    assert_equal(vec.dimensions(), 6);
    assert_equal(vec.indices() == std::vector<int>{0, 2, 4}, true);
    assert_equal(vec.values() == std::vector<float>{1, 2, 3}, true);
}

static void test_constructor_span() {
    SparseVector vec{std::span<const float>{{1, 0, 2, 0, 3, 0}}};
    assert_equal(vec.dimensions(), 6);
}

static void test_constructor_map() {
    std::unordered_map<int, float> map{{2, 2}, {4, 3}, {3, 0}, {0, 1}};
    SparseVector vec{map, 6};
    assert_equal(vec.dimensions(), 6);
    assert_equal(vec.indices() == std::vector<int>{0, 2, 4}, true);
    assert_equal(vec.values() == std::vector<float>{1, 2, 3}, true);

    assert_exception<std::invalid_argument>(
        [&] { SparseVector{map, -1}; }, "sparsevec cannot have negative dimensions"
    );

    assert_exception<std::invalid_argument>(
        [&] { SparseVector{map, 4}; }, "sparsevec index out of bounds"
    );

    assert_exception<std::invalid_argument>(
        [] { SparseVector{{{0, 1}}, 0}; }, "sparsevec index out of bounds"
    );
}

static void test_constructor_empty() {
    SparseVector vec{std::vector<float>{}};
    assert_equal(vec.dimensions(), 0);

    SparseVector vec2{{}, 0};
    assert_equal(vec2.dimensions(), 0);
}

static void test_dimensions() {
    SparseVector vec{std::vector<float>{1, 0, 2, 0, 3, 0}};
    assert_equal(vec.dimensions(), 6);
}

static void test_indices() {
    SparseVector vec{std::vector<float>{1, 0, 2, 0, 3, 0}};
    assert_equal(vec.indices() == std::vector<int>{0, 2, 4}, true);
}

static void test_values() {
    SparseVector vec{std::vector<float>{1, 0, 2, 0, 3, 0}};
    assert_equal(vec.values() == std::vector<float>{1, 2, 3}, true);
}

static void test_string() {
    SparseVector vec{std::vector<float>{1, 0, 2, 0, 3, 0}};
    std::ostringstream oss;
    oss << vec;
    assert_equal(oss.str(), "{1:1,3:2,5:3}/6");
}

void test_sparsevec() {
    test_constructor_vector();
    test_constructor_span();
    test_constructor_empty();
    test_constructor_map();
    test_dimensions();
    test_indices();
    test_values();
    test_string();
}
