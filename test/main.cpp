#include <cassert>

#include "../include/pgvector/halfvec.hpp"
#include "../include/pgvector/sparsevec.hpp"
#include "../include/pgvector/vector.hpp"

using pgvector::HalfVector;
using pgvector::SparseVector;
using pgvector::Vector;

void test_pqxx();

void test_vector() {
    auto vec = Vector({1, 2, 3});
    assert(vec.dimensions() == 3);
}

void test_halfvec() {
    auto vec = HalfVector({1, 2, 3});
    assert(vec.dimensions() == 3);
}

void test_sparsevec() {
    auto vec = SparseVector({1, 2, 3});
    assert(vec.dimensions() == 3);
}

int main() {
    test_pqxx();
    test_vector();
    test_halfvec();
    test_sparsevec();
    return 0;
}
