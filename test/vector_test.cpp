#include <span>
#include <vector>

#include <pgvector/vector.hpp>

#include "helper.hpp"

using pgvector::Vector;

static void test_constructor_vector() {
    Vector vec{std::vector<float>{1, 2, 3}};
    assert_equal(vec.dimensions(), 3u);
}

static void test_constructor_span() {
    Vector vec{std::span<const float>{{1, 2, 3}}};
    assert_equal(vec.dimensions(), 3u);
}

static void test_constructor_empty() {
    Vector vec{std::vector<float>{}};
    assert_equal(vec.dimensions(), 0u);
}

static void test_values() {
    Vector vec{{1, 2, 3}};
    assert_equal(vec.values() == std::vector<float>{1, 2, 3}, true);
}

void test_vector() {
    test_constructor_vector();
    test_constructor_span();
    test_constructor_empty();
    test_values();
}
