#include <span>
#include <sstream>
#include <vector>

#include <pgvector/vector.hpp>

#include "helper.hpp"

using pgvector::Vector;

namespace {
void test_constructor_vector() {
    Vector vec{std::vector<float>{1, 2, 3}};
    assert_equal(vec.dimensions(), 3u);
}

void test_constructor_span() {
    Vector vec{std::span<const float>{{1, 2, 3}}};
    assert_equal(vec.dimensions(), 3u);
}

void test_constructor_empty() {
    Vector vec{std::vector<float>{}};
    assert_equal(vec.dimensions(), 0u);
}

void test_dimensions() {
    Vector vec{{1, 2, 3}};
    assert_equal(vec.dimensions(), 3u);
}

void test_values() {
    Vector vec{{1, 2, 3}};
    assert_equal(vec.values() == std::vector<float>{1, 2, 3}, true);
}

void test_string() {
    Vector vec{{1, 2, 3}};
    std::ostringstream oss;
    oss << vec;
    assert_equal(oss.str(), "[1,2,3]");
}
} // namespace

void test_vector() {
    test_constructor_vector();
    test_constructor_span();
    test_constructor_empty();
    test_dimensions();
    test_values();
    test_string();
}
