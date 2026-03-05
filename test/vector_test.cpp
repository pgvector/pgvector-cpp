#include <span>

#include <pgvector/vector.hpp>

#include "helper.hpp"

using pgvector::Vector;

static void test_constructor_vector() {
    auto vec = Vector({1, 2, 3});
    assert_equal(vec.dimensions(), 3u);
}

static void test_constructor_span() {
    auto vec = Vector(std::span<const float>({1, 2, 3}));
    assert_equal(vec.dimensions(), 3u);
}

static void test_conversion() {
    auto vec = Vector({1, 2, 3});
    auto float_vec = static_cast<std::vector<float>>(vec);
    assert_equal(float_vec == std::vector<float>{1, 2, 3}, true);
}

void test_vector() {
    test_constructor_vector();
    test_constructor_span();
    test_conversion();
}
