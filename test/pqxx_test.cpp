#include "../include/pqxx.hpp"
#include <cassert>
#include <optional>
#include <pqxx/pqxx>

void setup(pqxx::connection &conn) {
  pqxx::work txn{conn};
  txn.exec0("CREATE EXTENSION IF NOT EXISTS vector");
  txn.exec0("DROP TABLE IF EXISTS items");
  txn.exec0("CREATE TABLE items (id serial primary key, embedding vector(3))");
  txn.commit();
}

void test_works(pqxx::connection &conn) {
  pqxx::work txn{conn};
  auto embedding = pgvector::Vector({1, 2, 3});
  assert(embedding.dimensions() == 3);
  float arr[] = {4, 5, 6};
  auto embedding2 = pgvector::Vector(arr, 3);
  txn.exec_params("INSERT INTO items (embedding) VALUES ($1), ($2), ($3)",
                  embedding, embedding2, std::nullopt);

  pqxx::result res{txn.exec_params(
      "SELECT embedding FROM items ORDER BY embedding <-> $1", embedding2)};
  assert(res.size() == 3);
  assert(res[0][0].as<pgvector::Vector>() == embedding2);
  assert(res[1][0].as<pgvector::Vector>() == embedding);
  assert(!res[2][0].as<std::optional<pgvector::Vector>>().has_value());
  txn.commit();
}

void test_stream(pqxx::connection &conn) {
  pqxx::work txn{conn};
  auto stream = pqxx::stream_from::query(txn, "SELECT id, embedding FROM items WHERE embedding IS NOT NULL");
  std::tuple<int, pgvector::Vector> row;
  while (stream >> row) {
    assert(std::get<1>(row).dimensions() == 3);
  }
  stream.complete();
  txn.commit();
}

void test_precision(pqxx::connection &conn) {
  pqxx::work txn{conn};
  auto embedding = pgvector::Vector({1.23456789, 0, 0});
  txn.exec_params("INSERT INTO items (embedding) VALUES ($1)", embedding);
  txn.exec0("SET extra_float_digits = 3");
  pqxx::result res{txn.exec_params("SELECT embedding FROM items ORDER BY id DESC LIMIT 1")};
  assert(res[0][0].as<pgvector::Vector>() == embedding);
  txn.commit();
}

int main() {
  pqxx::connection conn("dbname=pgvector_cpp_test");
  setup(conn);

  test_works(conn);
  test_stream(conn);
  test_precision(conn);

  return 0;
}
