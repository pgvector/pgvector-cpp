#include <cassert>
#include <optional>
#include <string>
#include <vector>

#include <pqxx/pqxx>

#include "../include/pgvector/pqxx.hpp"

void setup(pqxx::connection &conn) {
    pqxx::nontransaction tx(conn);
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");
    tx.exec("DROP TABLE IF EXISTS items");
    tx.exec("CREATE TABLE items (id serial PRIMARY KEY, embedding vector(3), half_embedding halfvec(3), binary_embedding bit(3), sparse_embedding sparsevec(3))");
}

void before_each(pqxx::connection &conn) {
    pqxx::nontransaction tx(conn);
    tx.exec("TRUNCATE items");
}

void test_vector(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::Vector({1, 2, 3});
    assert(embedding.dimensions() == 3);
    float arr[] = {4, 5, 6};
    auto embedding2 = pgvector::Vector(arr, 3);
    tx.exec("INSERT INTO items (embedding) VALUES ($1), ($2), ($3)", {embedding, embedding2, std::nullopt});

    pqxx::result res = tx.exec("SELECT embedding FROM items ORDER BY embedding <-> $1", {embedding2});
    assert(res.size() == 3);
    assert(res[0][0].as<pgvector::Vector>() == embedding2);
    assert(res[1][0].as<pgvector::Vector>() == embedding);
    assert(!res[2][0].as<std::optional<pgvector::Vector>>().has_value());

    assert(pqxx::to_string(embedding) == "[1,2,3]");
    assert(pqxx::from_string<pgvector::Vector>("[1,2,3]") == embedding);
}

void test_halfvec(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::HalfVector({1, 2, 3});
    assert(embedding.dimensions() == 3);
    float arr[] = {4, 5, 6};
    auto embedding2 = pgvector::HalfVector(arr, 3);
    tx.exec("INSERT INTO items (half_embedding) VALUES ($1), ($2), ($3)", {embedding, embedding2, std::nullopt});

    pqxx::result res = tx.exec("SELECT half_embedding FROM items ORDER BY half_embedding <-> $1", {embedding2});
    assert(res.size() == 3);
    assert(res[0][0].as<pgvector::HalfVector>() == embedding2);
    assert(res[1][0].as<pgvector::HalfVector>() == embedding);
    assert(!res[2][0].as<std::optional<pgvector::HalfVector>>().has_value());

    assert(pqxx::to_string(embedding) == "[1,2,3]");
    assert(pqxx::from_string<pgvector::HalfVector>("[1,2,3]") == embedding);
}

void test_bit(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = "101";
    auto embedding2 = "111";
    tx.exec("INSERT INTO items (binary_embedding) VALUES ($1), ($2), ($3)", {embedding, embedding2, std::nullopt});

    pqxx::result res = tx.exec("SELECT binary_embedding FROM items ORDER BY binary_embedding <~> $1", pqxx::params{embedding2});
    assert(res.size() == 3);
    assert(res[0][0].as<std::string>() == embedding2);
    assert(res[1][0].as<std::string>() == embedding);
    assert(!res[2][0].as<std::optional<std::string>>().has_value());
}

void test_sparsevec(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::SparseVector({1, 2, 3});
    auto embedding2 = pgvector::SparseVector({4, 5, 6});
    tx.exec("INSERT INTO items (sparse_embedding) VALUES ($1), ($2), ($3)", {embedding, embedding2, std::nullopt});

    pqxx::result res = tx.exec("SELECT sparse_embedding FROM items ORDER BY sparse_embedding <-> $1", {embedding2});
    assert(res.size() == 3);
    assert(res[0][0].as<std::string>() == "{1:4,2:5,3:6}/3");
    assert(res[1][0].as<std::string>() == "{1:1,2:2,3:3}/3");
    assert(!res[2][0].as<std::optional<std::string>>().has_value());

    assert(pqxx::to_string(embedding) == "{1:1,2:2,3:3}/3");
    // TODO add
    // assert(pqxx::from_string<pgvector::SparseVector>("{1:1,2:2,3:3}/3") == embedding);
}

void test_sparsevec_nnz(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    std::vector<float> vec(16001, 1);
    auto embedding = pgvector::SparseVector(vec);
    try {
        tx.exec("INSERT INTO items (sparse_embedding) VALUES ($1)", {embedding});
        assert(false);
    } catch (const pqxx::conversion_overrun& e) {
        assert(std::strcmp(e.what(), "sparsevec cannot have more than 16000 dimensions") == 0);
    }
}

void test_stream(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::Vector({1, 2, 3});
    tx.exec("INSERT INTO items (embedding) VALUES ($1)", {embedding});
    int count = 0;
    for (auto [id, embedding] : tx.stream<int, pgvector::Vector>("SELECT id, embedding FROM items WHERE embedding IS NOT NULL")) {
        assert(embedding.dimensions() == 3);
        count++;
    }
    assert(count == 1);
}

void test_stream_to(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto stream = pqxx::stream_to::table(tx, {"items"}, {"embedding"});
    stream << pgvector::Vector({1, 2, 3});
    stream << pgvector::Vector({4, 5, 6});
    stream.complete();
    pqxx::result res = tx.exec("SELECT embedding FROM items ORDER BY id");
    assert(res[0][0].as<std::string>() == "[1,2,3]");
    assert(res[1][0].as<std::string>() == "[4,5,6]");
}

void test_precision(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::Vector({1.23456789, 0, 0});
    tx.exec("INSERT INTO items (embedding) VALUES ($1)", {embedding});
    tx.exec("SET extra_float_digits = 3");
    pqxx::result res = tx.exec("SELECT embedding FROM items ORDER BY id DESC LIMIT 1");
    assert(res[0][0].as<pgvector::Vector>() == embedding);
}

void test_pqxx() {
    pqxx::connection conn("dbname=pgvector_cpp_test");
    setup(conn);

    test_vector(conn);
    test_halfvec(conn);
    test_bit(conn);
    test_sparsevec(conn);
    test_sparsevec_nnz(conn);
    test_stream(conn);
    test_stream_to(conn);
    test_precision(conn);
}
