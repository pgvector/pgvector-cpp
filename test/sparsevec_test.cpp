#include <span>
#include <unordered_map>

#include <pgvector/sparsevec.hpp>

#include "helper.hpp"

using pgvector::SparseVector;

static void test_constructor_vector() {
    auto vec = SparseVector(std::vector<float>{1, 0, 2, 0, 3, 0});
    assert_equal(vec.dimensions(), 6);
    assert_equal(vec.indices() == std::vector<int>{0, 2, 4}, true);
    assert_equal(vec.values() == std::vector<float>{1, 2, 3}, true);
}

static void test_constructor_span() {
    auto vec = SparseVector(std::span<const float>({1, 0, 2, 0, 3, 0}));
    assert_equal(vec.dimensions(), 6);
}

static void test_constructor_map() {
    std::unordered_map<int, float> map = {{2, 2}, {4, 3}, {3, 0}, {0, 1}};
    auto vec = SparseVector(map, 6);
    assert_equal(vec.dimensions(), 6);
    assert_equal(vec.indices() == std::vector<int>{0, 2, 4}, true);
    assert_equal(vec.values() == std::vector<float>{1, 2, 3}, true);

    assert_exception<std::invalid_argument>([&]{
        SparseVector(map, 0);
    }, "sparsevec must have at least 1 dimension");

    assert_exception<std::invalid_argument>([&]{
        SparseVector(map, 4);
    }, "sparsevec index out of bounds");
}

void test_sparsevec() {
    test_constructor_vector();
    test_constructor_span();
    test_constructor_map();
}
