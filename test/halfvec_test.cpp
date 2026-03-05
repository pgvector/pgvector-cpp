#include <span>

#include <pgvector/halfvec.hpp>

#include "helper.hpp"

using pgvector::HalfVector;

static void test_constructor_vector() {
    auto vec = HalfVector(std::vector<pgvector::Half>{1, 2, 3});
    assert_equal(vec.dimensions(), 3u);
}

static void test_constructor_span() {
    auto vec = HalfVector(std::span<const pgvector::Half>({1, 2, 3}));
    assert_equal(vec.dimensions(), 3u);
}

static void test_as_vector() {
    auto vec = HalfVector({1, 2, 3});
    auto& half_vec = vec.as_vector();
    assert_equal(half_vec == std::vector<pgvector::Half>{1, 2, 3}, true);
}

void test_halfvec() {
    test_constructor_vector();
    test_constructor_span();
    test_as_vector();
}
