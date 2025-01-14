// good resources
// https://opensearch.org/blog/improving-document-retrieval-with-sparse-semantic-encoders/
// https://huggingface.co/opensearch-project/opensearch-neural-sparse-encoding-v1
//
// run with
// text-embeddings-router --model-id opensearch-project/opensearch-neural-sparse-encoding-v1 --pooling splade

#include <cstdint>
#include <iostream>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <pgvector/pqxx.hpp>
#include <pqxx/pqxx>

using json = nlohmann::json;

std::vector<pgvector::SparseVector> fetch_embeddings(const std::vector<std::string>& inputs) {
    std::string url = "http://localhost:3000/embed_sparse";
    json data = {
        {"inputs", inputs}
    };

    cpr::Response r = cpr::Post(
        cpr::Url{url},
        cpr::Body{data.dump()},
        cpr::Header{{"Content-Type", "application/json"}}
    );
    json response = json::parse(r.text);

    std::vector<pgvector::SparseVector> embeddings;
    for (auto& item : response) {
        std::vector<int> indices;
        std::vector<float> values;
        for (auto& e : item) {
            indices.emplace_back(e["index"]);
            values.emplace_back(e["value"]);
        }
        embeddings.emplace_back(pgvector::SparseVector(30522, indices, values));
    }
    return embeddings;
}

int main() {
    pqxx::connection conn("dbname=pgvector_example");

    pqxx::nontransaction tx(conn);
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");
    tx.exec("DROP TABLE IF EXISTS documents");
    tx.exec("CREATE TABLE documents (id bigserial PRIMARY KEY, content text, embedding sparsevec(30522))");

    std::vector<std::string> input = {
        "The dog is barking",
        "The cat is purring",
        "The bear is growling"
    };
    auto embeddings = fetch_embeddings(input);

    for (size_t i = 0; i < input.size(); i++) {
        tx.exec("INSERT INTO documents (content, embedding) VALUES ($1, $2)", pqxx::params{input[i], embeddings[i]});
    }

    std::string query = "forest";
    auto query_embedding = fetch_embeddings({query})[0];
    pqxx::result result = tx.exec("SELECT content FROM documents ORDER BY embedding <#> $1 LIMIT 5", pqxx::params{query_embedding});
    for (const auto& row : result) {
        std::cout << row[0].as<std::string>() << std::endl;
    }

    return 0;
}
