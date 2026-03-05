#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <pqxx/pqxx>
#include <pgvector/pqxx.hpp>

#include "helper.hpp"

using namespace std::string_literals;

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

std::optional<std::string_view> float_error([[maybe_unused]] std::string_view message) {
#ifdef __linux__
    return message;
#else
    return std::nullopt;
#endif
}

void test_vector(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::Vector({1, 2, 3});
    float arr[] = {4, 5, 6};
    auto embedding2 = pgvector::Vector(std::span{arr, 3});
    tx.exec("INSERT INTO items (embedding) VALUES ($1), ($2), ($3)", {embedding, embedding2, std::nullopt});

    pqxx::result res = tx.exec("SELECT embedding FROM items ORDER BY embedding <-> $1", {embedding2});
    assert_equal(res.size(), 3);
    assert_equal(res[0][0].as<pgvector::Vector>(), embedding2);
    assert_equal(res[1][0].as<pgvector::Vector>(), embedding);
    assert_true(!res[2][0].as<std::optional<pgvector::Vector>>().has_value());
}

void test_halfvec(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::HalfVector({1, 2, 3});
    float arr[] = {4, 5, 6};
    auto embedding2 = pgvector::HalfVector(std::span{arr, 3});
    tx.exec("INSERT INTO items (half_embedding) VALUES ($1), ($2), ($3)", {embedding, embedding2, std::nullopt});

    pqxx::result res = tx.exec("SELECT half_embedding FROM items ORDER BY half_embedding <-> $1", {embedding2});
    assert_equal(res.size(), 3);
    assert_equal(res[0][0].as<pgvector::HalfVector>(), embedding2);
    assert_equal(res[1][0].as<pgvector::HalfVector>(), embedding);
    assert_true(!res[2][0].as<std::optional<pgvector::HalfVector>>().has_value());
}

void test_bit(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    std::string embedding = "101";
    std::string embedding2 = "111";
    tx.exec("INSERT INTO items (binary_embedding) VALUES ($1), ($2), ($3)", {embedding, embedding2, std::nullopt});

    pqxx::result res = tx.exec("SELECT binary_embedding FROM items ORDER BY binary_embedding <~> $1", pqxx::params{embedding2});
    assert_equal(res.size(), 3);
    assert_equal(res[0][0].as<std::string>(), embedding2);
    assert_equal(res[1][0].as<std::string>(), embedding);
    assert_true(!res[2][0].as<std::optional<std::string>>().has_value());
}

void test_sparsevec(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::SparseVector({1, 2, 3});
    auto embedding2 = pgvector::SparseVector({4, 5, 6});
    tx.exec("INSERT INTO items (sparse_embedding) VALUES ($1), ($2), ($3)", {embedding, embedding2, std::nullopt});

    pqxx::result res = tx.exec("SELECT sparse_embedding FROM items ORDER BY sparse_embedding <-> $1", {embedding2});
    assert_equal(res.size(), 3);
    assert_equal(res[0][0].as<pgvector::SparseVector>(), embedding2);
    assert_equal(res[1][0].as<pgvector::SparseVector>(), embedding);
    assert_true(!res[2][0].as<std::optional<pgvector::SparseVector>>().has_value());
}

void test_sparsevec_nnz(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    std::vector<float> vec(16001, 1);
    auto embedding = pgvector::SparseVector(vec);
    assert_exception<pqxx::conversion_overrun>([&] {
        tx.exec("INSERT INTO items (sparse_embedding) VALUES ($1)", {embedding});
    }, "sparsevec cannot have more than 16000 dimensions");
}

void test_stream(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::Vector({1, 2, 3});
    tx.exec("INSERT INTO items (embedding) VALUES ($1)", {embedding});
    int count = 0;
    for (auto [id, embedding2] : tx.stream<int, pgvector::Vector>("SELECT id, embedding FROM items WHERE embedding IS NOT NULL")) {
        assert_equal(embedding2, embedding);
        count++;
    }
    assert_equal(count, 1);
}

void test_stream_to(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto stream = pqxx::stream_to::table(tx, {"items"}, {"embedding"});
    stream.write_values(pgvector::Vector({1, 2, 3}));
    stream.write_values(pgvector::Vector({4, 5, 6}));
    stream.complete();
    pqxx::result res = tx.exec("SELECT embedding FROM items ORDER BY id");
    assert_equal(res[0][0].as<std::string>(), "[1,2,3]"s);
    assert_equal(res[1][0].as<std::string>(), "[4,5,6]"s);
}

void test_precision(pqxx::connection &conn) {
    before_each(conn);

    pqxx::nontransaction tx(conn);
    auto embedding = pgvector::Vector({1.23456789, 0, 0});
    tx.exec("INSERT INTO items (embedding) VALUES ($1)", {embedding});
    tx.exec("SET extra_float_digits = 3");
    pqxx::result res = tx.exec("SELECT embedding FROM items ORDER BY id DESC LIMIT 1");
    assert_equal(res[0][0].as<pgvector::Vector>(), embedding);
}

void test_vector_to_string() {
    assert_equal(pqxx::to_string(pgvector::Vector({1, 2, 3})), "[1,2,3]"s);
    assert_equal(pqxx::to_string(pgvector::Vector({-1.234567890123})), "[-1.2345679]"s);

    assert_exception<pqxx::conversion_overrun>([] {
        auto unused = pqxx::to_string(pgvector::Vector(std::vector<float>(16001)));
    }, "vector cannot have more than 16000 dimensions");
}

void test_vector_from_string() {
    assert_equal(pqxx::from_string<pgvector::Vector>("[1,2,3]"), pgvector::Vector({1, 2, 3}));
    assert_equal(pqxx::from_string<pgvector::Vector>("[]"), pgvector::Vector(std::vector<float>{}));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::Vector>("");
    }, "Malformed vector literal");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::Vector>("[");
    }, "Malformed vector literal");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::Vector>("[hello]");
    }, float_error("Could not convert 'hello' to float: Invalid argument."));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::Vector>("[4e38]");
    }, float_error("Could not convert '4e38' to float: Value out of range."));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::Vector>("[,]");
    }, float_error("Could not convert '' to float: Invalid argument."));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::Vector>("[1,]");
    }, float_error("Could not convert '' to float: Invalid argument."));
}

void test_halfvec_to_string() {
    assert_equal(pqxx::to_string(pgvector::HalfVector({1, 2, 3})), "[1,2,3]"s);
    assert_equal(pqxx::to_string(pgvector::HalfVector({-1.234567890123})), "[-1.2345679]"s);

    assert_exception<pqxx::conversion_overrun>([] {
        auto unused = pqxx::to_string(pgvector::HalfVector(std::vector<float>(16001)));
    }, "halfvec cannot have more than 16000 dimensions");
}

void test_halfvec_from_string() {
    assert_equal(pqxx::from_string<pgvector::HalfVector>("[1,2,3]"), pgvector::HalfVector({1, 2, 3}));
    assert_equal(pqxx::from_string<pgvector::HalfVector>("[]"), pgvector::HalfVector(std::vector<float>{}));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::HalfVector>("");
    }, "Malformed halfvec literal");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::HalfVector>("[");
    }, "Malformed halfvec literal");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::HalfVector>("[hello]");
    }, float_error("Could not convert 'hello' to float: Invalid argument."));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::HalfVector>("[4e38]");
    }, float_error("Could not convert '4e38' to float: Value out of range."));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::HalfVector>("[,]");
    }, float_error("Could not convert '' to float: Invalid argument."));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::HalfVector>("[1,]");
    }, float_error("Could not convert '' to float: Invalid argument."));
}

void test_sparsevec_to_string() {
    assert_equal(pqxx::to_string(pgvector::SparseVector({1, 0, 2, 0, 3, 0})), "{1:1,3:2,5:3}/6"s);
    std::unordered_map<int, float> map = {{999999999, -1.234567890123}};
    assert_equal(pqxx::to_string(pgvector::SparseVector(map, 1000000000)), "{1000000000:-1.2345679}/1000000000"s);
}

void test_sparsevec_from_string() {
    assert_equal(pqxx::from_string<pgvector::SparseVector>("{1:1,3:2,5:3}/6"), pgvector::SparseVector({1, 0, 2, 0, 3, 0}));
    assert_equal(pqxx::from_string<pgvector::SparseVector>("{}/6"), pgvector::SparseVector({0, 0, 0, 0, 0, 0}));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("");
    }, "Malformed sparsevec literal");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{");
    }, "Malformed sparsevec literal");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{ }/");
    }, "Could not convert '' to int: Invalid argument.");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{}/-1");
    }, "Dimensions cannot be negative");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{:}/1");
    }, "Could not convert '' to int: Invalid argument.");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{,}/1");
    }, "Malformed sparsevec literal");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{0:1}/1");
    }, "Index out of bounds");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{-2147483648:1}/1");
    }, "Index out of bounds");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{1:4e38}/1");
    }, float_error("Could not convert '4e38' to float: Value out of range."));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{a:1}/1");
    }, "Could not convert 'a' to int: Invalid argument.");

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{1:a}/1");
    }, float_error("Could not convert 'a' to float: Invalid argument."));

    assert_exception<pqxx::conversion_error>([] {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{}/a");
    }, "Could not convert 'a' to int: Invalid argument.");

    assert_exception<pqxx::conversion_overrun>([] {
        auto unused = pqxx::to_string(pgvector::SparseVector(std::vector<float>(16001, 1)));
    }, "sparsevec cannot have more than 16000 dimensions");
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

    test_vector_to_string();
    test_vector_from_string();
    test_halfvec_to_string();
    test_halfvec_from_string();
    test_sparsevec_to_string();
    test_sparsevec_from_string();
}
