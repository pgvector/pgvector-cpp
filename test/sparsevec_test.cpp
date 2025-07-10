#include <cassert>
#include <unordered_map>

#include <pgvector/sparsevec.hpp>

#if __cplusplus >= 202002L
#include <span>
#endif

using pgvector::SparseVector;

static void test_constructor_vector() {
    auto vec = SparseVector({1, 0, 2, 0, 3, 0});
    assert(vec.dimensions() == 6);
    assert(vec.indices() == (std::vector<int>{0, 2, 4}));
    assert(vec.values() == (std::vector<float>{1, 2, 3}));
}

#if __cplusplus >= 202002L
static void test_constructor_span() {
    auto vec = SparseVector(std::span<const float>({1, 0, 2, 0, 3, 0}));
    assert(vec.dimensions() == 6);
}
#endif

static void test_constructor_map() {
    std::unordered_map<int, float> map = {{2, 2}, {4, 3}, {3, 0}, {0, 1}};
    auto vec = SparseVector(map, 6);
    assert(vec.dimensions() == 6);
    assert(vec.indices() == (std::vector<int>{0, 2, 4}));
    assert(vec.values() == (std::vector<float>{1, 2, 3}));
}

void test_sparsevec() {
    test_constructor_vector();
#if __cplusplus >= 202002L
    test_constructor_span();
#endif
    test_constructor_map();
}
