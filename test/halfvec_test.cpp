#include <cassert>
#include <span>

#include <pgvector/halfvec.hpp>

using pgvector::HalfVector;

static void test_constructor_vector() {
    auto vec = HalfVector({1, 2, 3});
    assert(vec.dimensions() == 3);
}

static void test_constructor_span() {
    auto vec = HalfVector(std::span<const float>({1, 2, 3}));
    assert(vec.dimensions() == 3);
}

void test_halfvec() {
    test_constructor_vector();
    test_constructor_span();
}
