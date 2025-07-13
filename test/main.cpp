// Test ODR
#include <pgvector/pqxx.hpp>

void test_vector();
void test_halfvec();
void test_sparsevec();
void test_pqxx();

int main() {
    test_vector();
    test_halfvec();
    test_sparsevec();
    test_pqxx();
    return 0;
}
