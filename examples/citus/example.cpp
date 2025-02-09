#include <iostream>
#include <random>
#include <cstdint>

#include <pgvector/pqxx.hpp>
#include <pqxx/pqxx>

std::vector<std::vector<float>> random_embeddings(int rows, int dimensions) {
    std::random_device rd;
    std::mt19937_64 prng(rd());
    std::uniform_real_distribution<float> dist(0, 1);

    std::vector<std::vector<float>> embeddings;
    embeddings.reserve(rows);
    for (int i = 0; i < rows; i++) {
        std::vector<float> embedding;
        embedding.reserve(dimensions);
        for (int j = 0; j < dimensions; j++) {
            embedding.push_back(dist(prng));
        }
        embeddings.push_back(embedding);
    }
    return embeddings;
}

std::vector<int64_t> random_categories(int rows) {
    std::random_device rd;
    std::mt19937_64 prng(rd());
    std::uniform_int_distribution<int64_t> dist(1, 100);

    std::vector<int64_t> categories;
    categories.reserve(rows);
    for (int i = 0; i < rows; i++) {
        categories.push_back(dist(prng));
    }
    return categories;
}

int main() {
    // generate random data
    int rows = 100000;
    int dimensions = 128;
    auto embeddings = random_embeddings(rows, dimensions);
    auto categories = random_categories(rows);
    auto queries = random_embeddings(10, dimensions);

    // enable extensions
    pqxx::connection conn("dbname=pgvector_citus");
    pqxx::nontransaction tx(conn);
    tx.exec("CREATE EXTENSION IF NOT EXISTS citus");
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");

    // GUC variables set on the session do not propagate to Citus workers
    // https://github.com/citusdata/citus/issues/462
    // you can either:
    // 1. set them on the system, user, or database and reconnect
    // 2. set them for a transaction with SET LOCAL
    tx.exec("ALTER DATABASE pgvector_citus SET maintenance_work_mem = '512MB'");
    tx.exec("ALTER DATABASE pgvector_citus SET hnsw.ef_search = 20");
    conn.close();

    // reconnect for updated GUC variables to take effect
    pqxx::connection conn2("dbname=pgvector_citus");
    pqxx::nontransaction tx2(conn2);

    std::cout << "Creating distributed table" << std::endl;
    tx2.exec("DROP TABLE IF EXISTS items");
    tx2.exec("CREATE TABLE items (id bigserial, embedding vector(128), category_id bigint, PRIMARY KEY (id, category_id))");
    tx2.exec("SET citus.shard_count = 4");
    tx2.exec("SELECT create_distributed_table('items', 'category_id')");

    // libpqxx does not support binary COPY
    std::cout << "Loading data in parallel" << std::endl;
    auto stream = pqxx::stream_to::table(tx2, {"items"}, {"embedding", "category_id"});
    for (size_t i = 0; i < embeddings.size(); i++) {
        stream << std::make_tuple(pgvector::Vector(embeddings[i]), categories[i]);
    }
    stream.complete();

    std::cout << "Creating index in parallel" << std::endl;
    tx2.exec("CREATE INDEX ON items USING hnsw (embedding vector_l2_ops)");

    std::cout << "Running distributed queries" << std::endl;
    for (auto& query : queries) {
        pqxx::result result = tx2.exec(
            "SELECT id FROM items ORDER BY embedding <-> $1 LIMIT 10",
            pqxx::params{pgvector::Vector(query)}
        );
        for (const auto& row : result) {
            std::cout << row[0].as<int64_t>() << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
