#include <span>

#include <pgvector/halfvec.hpp>

#include "helper.hpp"

using pgvector::HalfVector;

static void test_constructor_vector() {
    auto vec = HalfVector({1, 2, 3});
    assert_equal(vec.dimensions(), 3u);
}

static void test_constructor_span() {
    auto vec = HalfVector(std::span<const float>({1, 2, 3}));
    assert_equal(vec.dimensions(), 3u);
}

void test_halfvec() {
    test_constructor_vector();
    test_constructor_span();
}
