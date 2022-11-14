#include "../include/pqxx.hpp"
#include <cassert>
#include <optional>
#include <pqxx/pqxx>

void setup(pqxx::connection &conn) {
  pqxx::work txn{conn};
  txn.exec0("CREATE EXTENSION IF NOT EXISTS vector");
  txn.exec0("DROP TABLE IF EXISTS items");
  txn.exec0("CREATE TABLE items (id serial primary key, factors vector(3))");
  txn.commit();
}

void test_works(pqxx::connection &conn) {
  pqxx::work txn{conn};
  auto factors = pgvector::Vector({1, 2, 3});
  assert(factors.dimensions() == 3);
  float arr[] = {4, 5, 6};
  auto factors2 = pgvector::Vector(arr, 3);
  txn.exec_params("INSERT INTO items (factors) VALUES ($1), ($2), ($3)",
                  factors, factors2, std::nullopt);

  pqxx::result res{txn.exec_params(
      "SELECT factors FROM items ORDER BY factors <-> $1", factors2)};
  assert(res.size() == 3);
  assert(res[0][0].as<pgvector::Vector>() == factors2);
  assert(res[1][0].as<pgvector::Vector>() == factors);
  assert(!res[2][0].as<std::optional<pgvector::Vector>>().has_value());
  txn.commit();
}

int main() {
  pqxx::connection conn("dbname=pgvector_cpp_test");
  setup(conn);

  test_works(conn);

  return 0;
}
