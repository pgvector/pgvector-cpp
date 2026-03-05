#include <span>

#include <pgvector/vector.hpp>

#include "helper.hpp"

using pgvector::Vector;

static void test_constructor_vector() {
    auto vec = Vector({1, 2, 3});
    assert_equal(vec.dimensions(), static_cast<size_t>(3));
}

static void test_constructor_span() {
    auto vec = Vector(std::span<const float>({1, 2, 3}));
    assert_equal(vec.dimensions(), static_cast<size_t>(3));
}

void test_vector() {
    test_constructor_vector();
    test_constructor_span();
}
