#include <span>

#include <pgvector/halfvec.hpp>

#include "helper.hpp"

using pgvector::HalfVector;

static void test_constructor_vector() {
    HalfVector vec{std::vector<pgvector::Half>{1, 2, 3}};
    assert_equal(vec.dimensions(), 3u);
}

static void test_constructor_span() {
    HalfVector vec{std::span<const pgvector::Half>{{1, 2, 3}}};
    assert_equal(vec.dimensions(), 3u);
}

static void test_as_vector() {
    HalfVector vec{{1, 2, 3}};
    assert_equal(vec.as_vector() == std::vector<pgvector::Half>{1, 2, 3}, true);
}

void test_halfvec() {
    test_constructor_vector();
    test_constructor_span();
    test_as_vector();
}
