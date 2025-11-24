#include <cassert>
#include <optional>
#include <string>
#include <vector>

#include <pqxx/pqxx>

#include <pgvector/pqxx.hpp>

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
    assert(res[0][0].as<pgvector::SparseVector>() == embedding2);
    assert(res[1][0].as<pgvector::SparseVector>() == embedding);
    assert(!res[2][0].as<std::optional<pgvector::SparseVector>>().has_value());
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
        assert(std::string_view(e.what()) == "sparsevec cannot have more than 16000 dimensions");
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
    stream.write_values(pgvector::Vector({1, 2, 3}));
    stream.write_values(pgvector::Vector({4, 5, 6}));
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

void test_vector_to_string() {
    assert(pqxx::to_string(pgvector::Vector({1, 2, 3})) == "[1,2,3]");
}

void test_vector_from_string() {
    assert(pqxx::from_string<pgvector::Vector>("[1,2,3]") == pgvector::Vector({1, 2, 3}));

    try {
        auto unused = pqxx::from_string<pgvector::Vector>("");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Malformed vector literal");
    }

    try {
        auto unused = pqxx::from_string<pgvector::Vector>("[");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Malformed vector literal");
    }

    // TODO change to no error?
    try {
        auto unused = pqxx::from_string<pgvector::Vector>("[]");
        assert(false);
    } catch (const std::invalid_argument& e) {
        assert(true);
    }

    // TODO change to pqxx::conversion_error
    try {
        auto unused = pqxx::from_string<pgvector::Vector>("[hello]");
        assert(false);
    } catch (const std::invalid_argument& e) {
        assert(true);
    }

    // TODO change to pqxx::conversion_error
    try {
        auto unused = pqxx::from_string<pgvector::Vector>("[4e38]");
        assert(false);
    } catch (const std::out_of_range& e) {
        assert(true);
    }
}

void test_halfvec_to_string() {
    assert(pqxx::to_string(pgvector::HalfVector({1, 2, 3})) == "[1,2,3]");
}

void test_halfvec_from_string() {
    assert(pqxx::from_string<pgvector::HalfVector>("[1,2,3]") == pgvector::HalfVector({1, 2, 3}));

    try {
        auto unused = pqxx::from_string<pgvector::HalfVector>("");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Malformed halfvec literal");
    }

    try {
        auto unused = pqxx::from_string<pgvector::HalfVector>("[");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Malformed halfvec literal");
    }

    // TODO change to no error?
    try {
        auto unused = pqxx::from_string<pgvector::HalfVector>("[]");
        assert(false);
    } catch (const std::invalid_argument& e) {
        assert(true);
    }

    // TODO change to pqxx::conversion_error
    try {
        auto unused = pqxx::from_string<pgvector::HalfVector>("[hello]");
        assert(false);
    } catch (const std::invalid_argument& e) {
        assert(true);
    }
}

void test_sparsevec_to_string() {
    assert(pqxx::to_string(pgvector::SparseVector({1, 0, 2, 0, 3, 0})) == "{1:1,3:2,5:3}/6");
}

void test_sparsevec_from_string() {
    assert(pqxx::from_string<pgvector::SparseVector>("{1:1,3:2,5:3}/6") == pgvector::SparseVector({1, 0, 2, 0, 3, 0}));
    assert(pqxx::from_string<pgvector::SparseVector>("{}/6") == pgvector::SparseVector({0, 0, 0, 0, 0, 0}));

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Malformed sparsevec literal");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Malformed sparsevec literal");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{ }/");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Could not convert '' to int: Invalid argument.");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{}/-1");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Dimensions cannot be negative");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{:}/1");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Could not convert '' to int: Invalid argument.");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{,}/1");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Malformed sparsevec literal");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{0:1}/1");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Index out of bounds");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{-2147483648:1}/1");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Index out of bounds");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{1:4e38}/1");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Could not convert string to numeric value: '4e38'." || std::string_view(e.what()) == "Could not convert '4e38' to float" || std::string_view(e.what()) == "Could not convert '4e38' to float: Value out of range.");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{a:1}/1");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Could not convert 'a' to int: Invalid argument.");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{1:a}/1");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Could not convert string to numeric value: 'a'." || std::string_view(e.what()) == "Could not convert 'a' to float" || std::string_view(e.what()) == "Could not convert 'a' to float: Invalid argument.");
    }

    try {
        auto unused = pqxx::from_string<pgvector::SparseVector>("{}/a");
        assert(false);
    } catch (const pqxx::conversion_error& e) {
        assert(std::string_view(e.what()) == "Could not convert 'a' to int: Invalid argument.");
    }
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
