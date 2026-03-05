#include <span>

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

static void test_as_vector() {
    Vector vec{{1, 2, 3}};
    assert_equal(vec.as_vector() == std::vector<float>{1, 2, 3}, true);
}

void test_vector() {
    test_constructor_vector();
    test_constructor_span();
    test_as_vector();
}
