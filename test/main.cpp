#include <cassert>
#include <unordered_map>

#include "../include/pgvector/halfvec.hpp"
#include "../include/pgvector/sparsevec.hpp"
#include "../include/pgvector/vector.hpp"

#if __cplusplus >= 202002L
#include <span>
#endif

using pgvector::HalfVector;
using pgvector::SparseVector;
using pgvector::Vector;

void test_pqxx();

void test_vector() {
    auto vec = Vector({1, 2, 3});
    assert(vec.dimensions() == 3);
}

#if __cplusplus >= 202002L
void test_vector_span() {
    auto vec = Vector(std::span<const float>({1, 2, 3}));
    assert(vec.dimensions() == 3);
}
#endif

void test_halfvec() {
    auto vec = HalfVector({1, 2, 3});
    assert(vec.dimensions() == 3);
}

#if __cplusplus >= 202002L
void test_halfvec_span() {
    auto vec = HalfVector(std::span<const float>({1, 2, 3}));
    assert(vec.dimensions() == 3);
}
#endif

void test_sparsevec() {
    auto vec = SparseVector({1, 0, 2, 0, 3, 0});
    assert(vec.dimensions() == 6);
    assert(vec.indices() == (std::vector<int>{0, 2, 4}));
    assert(vec.values() == (std::vector<float>{1, 2, 3}));
}

#if __cplusplus >= 202002L
void test_sparsevec_span() {
    auto vec = SparseVector(std::span<const float>({1, 0, 2, 0, 3, 0}));
    assert(vec.dimensions() == 6);
}
#endif

void test_sparsevec_map() {
    std::unordered_map<int, float> map = {{2, 2}, {4, 3}, {3, 0}, {0, 1}};
    auto vec = SparseVector(map, 6);
    assert(vec.dimensions() == 6);
    assert(vec.indices() == (std::vector<int>{0, 2, 4}));
    assert(vec.values() == (std::vector<float>{1, 2, 3}));
}

int main() {
    test_pqxx();
    test_vector();
#if __cplusplus >= 202002L
    test_vector_span();
#endif
    test_halfvec();
#if __cplusplus >= 202002L
    test_halfvec_span();
#endif
    test_sparsevec();
#if __cplusplus >= 202002L
    test_sparsevec_span();
#endif
    return 0;
}
