#include <cassert>

#include "../include/pgvector/vector.hpp"

#if __cplusplus >= 202002L
#include <span>
#endif

using pgvector::Vector;

static void test_constructor_vector() {
    auto vec = Vector({1, 2, 3});
    assert(vec.dimensions() == 3);
}

#if __cplusplus >= 202002L
static void test_constructor_span() {
    auto vec = Vector(std::span<const float>({1, 2, 3}));
    assert(vec.dimensions() == 3);
}
#endif

void test_vector() {
    test_constructor_vector();
#if __cplusplus >= 202002L
    test_constructor_span();
#endif
}
