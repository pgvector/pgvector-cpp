#include "../include/pqxx.hpp"
#include <cassert>
#include <optional>
#include <pqxx/pqxx>

void setup(pqxx::connection &conn) {
  pqxx::work tx{conn};
  tx.exec0("CREATE EXTENSION IF NOT EXISTS vector");
  tx.exec0("DROP TABLE IF EXISTS items");
  tx.exec0("CREATE TABLE items (id serial PRIMARY KEY, embedding vector(3), half_embedding halfvec(3))");
  tx.commit();
}

void test_works(pqxx::connection &conn) {
  pqxx::work tx{conn};
  auto embedding = pgvector::Vector({1, 2, 3});
  assert(embedding.dimensions() == 3);
  float arr[] = {4, 5, 6};
  auto embedding2 = pgvector::Vector(arr, 3);
  tx.exec_params("INSERT INTO items (embedding) VALUES ($1), ($2), ($3)",
                  embedding, embedding2, std::nullopt);

  pqxx::result res{tx.exec_params(
      "SELECT embedding FROM items ORDER BY embedding <-> $1", embedding2)};
  assert(res.size() == 3);
  assert(res[0][0].as<pgvector::Vector>() == embedding2);
  assert(res[1][0].as<pgvector::Vector>() == embedding);
  assert(!res[2][0].as<std::optional<pgvector::Vector>>().has_value());
  tx.commit();
}

void test_halfvec(pqxx::connection &conn) {
  pqxx::work tx{conn};
  auto embedding = pgvector::HalfVector({1, 2, 3});
  assert(embedding.dimensions() == 3);
  float arr[] = {4, 5, 6};
  auto embedding2 = pgvector::HalfVector(arr, 3);
  tx.exec_params("INSERT INTO items (half_embedding) VALUES ($1), ($2), ($3)",
                  embedding, embedding2, std::nullopt);

  pqxx::result res{tx.exec_params(
      "SELECT embedding FROM items ORDER BY half_embedding <-> $1", embedding2)};
  assert(res.size() == 3);
  assert(res[0][0].as<pgvector::HalfVector>() == embedding2);
  assert(res[1][0].as<pgvector::HalfVector>() == embedding);
  assert(!res[2][0].as<std::optional<pgvector::HalfVector>>().has_value());
  tx.commit();
}

void test_stream(pqxx::connection &conn) {
  pqxx::work tx{conn};
  int count = 0;
  for (auto [id, embedding] : tx.stream<int, pgvector::Vector>("SELECT id, embedding FROM items WHERE embedding IS NOT NULL")) {
    assert(embedding.dimensions() == 3);
    count++;
  }
  assert(count == 2);
  tx.commit();
}

void test_precision(pqxx::connection &conn) {
  pqxx::work tx{conn};
  auto embedding = pgvector::Vector({1.23456789, 0, 0});
  tx.exec_params("INSERT INTO items (embedding) VALUES ($1)", embedding);
  tx.exec0("SET extra_float_digits = 3");
  pqxx::result res{tx.exec_params("SELECT embedding FROM items ORDER BY id DESC LIMIT 1")};
  assert(res[0][0].as<pgvector::Vector>() == embedding);
  tx.commit();
}

int main() {
  pqxx::connection conn("dbname=pgvector_cpp_test");
  setup(conn);

  test_works(conn);
  test_stream(conn);
  test_precision(conn);

  return 0;
}
