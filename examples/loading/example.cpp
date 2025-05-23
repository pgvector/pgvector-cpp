#include <iostream>
#include <random>
#include <vector>

#include <pgvector/pqxx.hpp>
#include <pqxx/pqxx>

int main() {
    // generate random data
    int rows = 100000;
    int dimensions = 128;
    std::vector<std::vector<float>> embeddings;
    embeddings.reserve(rows);
    std::mt19937_64 prng;
    std::uniform_real_distribution<float> dist(0, 1);
    for (int i = 0; i < rows; i++) {
        std::vector<float> embedding;
        embedding.reserve(dimensions);
        for (int j = 0; j < dimensions; j++) {
            embedding.push_back(dist(prng));
        }
        embeddings.push_back(embedding);
    }

    // enable extension
    pqxx::connection conn("dbname=pgvector_example");
    pqxx::nontransaction tx(conn);
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");

    // create table
    tx.exec("DROP TABLE IF EXISTS items");
    tx.exec("CREATE TABLE items (id bigserial, embedding vector(128))");

    // load data
    // libpqxx does not support binary COPY
    std::cout << "Loading " << rows << " rows" << std::endl;
    auto stream = pqxx::stream_to::table(tx, {"items"}, {"embedding"});
    for (size_t i = 0; i < embeddings.size(); i++) {
        // show progress
        if (i % 10000 == 0) {
            std::cout << '.' << std::flush;
        }

        stream << pgvector::Vector(embeddings[i]);
    }
    stream.complete();
    std::cout << std::endl << "Success!" << std::endl;

    // create any indexes *after* loading initial data (skipping for this example)
    bool create_index = false;
    if (create_index) {
        std::cout << "Creating index" << std::endl;
        tx.exec("SET maintenance_work_mem = '8GB'");
        tx.exec("SET max_parallel_maintenance_workers = 7");
        tx.exec("CREATE INDEX ON items USING hnsw (embedding vector_cosine_ops)");
    }

    // update planner statistics for good measure
    tx.exec("ANALYZE items");

    return 0;
}
