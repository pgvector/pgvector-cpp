// run with
// llama-server -hf nomic-ai/nomic-embed-text-v1.5-GGUF --embedding --pooling mean

#include <cstdint>
#include <iostream>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <pgvector/pqxx.hpp>
#include <pqxx/pqxx>

using json = nlohmann::json;

std::vector<std::vector<float>> fetch_embeddings(const std::vector<std::string>& input) {
    std::string url = "http://localhost:8080/v1/embeddings";
    json data = {
        {"input", input}
    };

    cpr::Response r = cpr::Post(
        cpr::Url{url},
        cpr::Body{data.dump()},
        cpr::Header{{"Content-Type", "application/json"}}
    );
    json response = json::parse(r.text);

    std::vector<std::vector<float>> embeddings;
    for (auto& v: response["data"]) {
        embeddings.emplace_back(v["embedding"]);
    }
    return embeddings;
}

int main() {
    pqxx::connection conn("dbname=pgvector_example");

    pqxx::work tx(conn);
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");
    tx.exec("DROP TABLE IF EXISTS documents");
    tx.exec("CREATE TABLE documents (id bigserial PRIMARY KEY, content text, embedding vector(768))");
    tx.exec("CREATE INDEX ON documents USING GIN (to_tsvector('english', content))");
    tx.commit();

    std::vector<std::string> input = {
        "The dog is barking",
        "The cat is purring",
        "The bear is growling"
    };
    auto embeddings = fetch_embeddings(input);

    for (size_t i = 0; i < input.size(); i++) {
        tx.exec("INSERT INTO documents (content, embedding) VALUES ($1, $2)", pqxx::params{input[i], pgvector::Vector(embeddings[i])});
    }
    tx.commit();

    std::string sql = R"(
    WITH semantic_search AS (
        SELECT id, RANK () OVER (ORDER BY embedding <=> $2) AS rank
        FROM documents
        ORDER BY embedding <=> $2
        LIMIT 20
    ),
    keyword_search AS (
        SELECT id, RANK () OVER (ORDER BY ts_rank_cd(to_tsvector('english', content), query) DESC)
        FROM documents, plainto_tsquery('english', $1) query
        WHERE to_tsvector('english', content) @@ query
        ORDER BY ts_rank_cd(to_tsvector('english', content), query) DESC
        LIMIT 20
    )
    SELECT
        COALESCE(semantic_search.id, keyword_search.id) AS id,
        COALESCE(1.0 / ($3 + semantic_search.rank), 0.0) +
        COALESCE(1.0 / ($3 + keyword_search.rank), 0.0) AS score
    FROM semantic_search
    FULL OUTER JOIN keyword_search ON semantic_search.id = keyword_search.id
    ORDER BY score DESC
    LIMIT 5
    )";
    std::string query = "growling bear";
    auto query_embedding = fetch_embeddings({query})[0];
    double k = 60;
    pqxx::result result = tx.exec(sql, pqxx::params{query, pgvector::Vector(query_embedding), k});
    for (const auto& row : result) {
        std::cout << "document: " << row[0].as<std::string>() << ", RRF score: " << row[1].as<double>() << std::endl;
    }

    return 0;
}
