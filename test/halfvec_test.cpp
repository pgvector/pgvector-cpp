#include <span>
#include <sstream>
#include <vector>

#include <pgvector/halfvec.hpp>

#include "helper.hpp"

using pgvector::HalfVector;

namespace {
void test_constructor_vector() {
    HalfVector vec{std::vector<pgvector::Half>{1, 2, 3}};
    assert_equal(vec.dimensions(), 3u);
}

void test_constructor_span() {
    HalfVector vec{std::span<const pgvector::Half>{{1, 2, 3}}};
    assert_equal(vec.dimensions(), 3u);
}

void test_constructor_empty() {
    HalfVector vec{std::vector<pgvector::Half>{}};
    assert_equal(vec.dimensions(), 0u);
}

void test_dimensions() {
    HalfVector vec{{1, 2, 3}};
    assert_equal(vec.dimensions(), 3u);
}

void test_values() {
    HalfVector vec{{1, 2, 3}};
    assert_equal(vec.values() == std::vector<pgvector::Half>{1, 2, 3}, true);
}

void test_string() {
    HalfVector vec{{1, 2, 3}};
    std::ostringstream oss;
    oss << vec;
    assert_equal(oss.str(), "[1,2,3]");
}
} // namespace

void test_halfvec() {
    test_constructor_vector();
    test_constructor_span();
    test_constructor_empty();
    test_dimensions();
    test_values();
    test_string();
}
