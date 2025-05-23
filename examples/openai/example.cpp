#include <iostream>
#include <string>
#include <vector>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <pgvector/pqxx.hpp>
#include <pqxx/pqxx>

using json = nlohmann::json;

// https://platform.openai.com/docs/guides/embeddings/how-to-get-embeddings
// input can be an array with 2048 elements
std::vector<std::vector<float>> embed(const std::vector<std::string>& input, char *api_key) {
    std::string url = "https://api.openai.com/v1/embeddings";
    json data = {
        {"input", input},
        {"model", "text-embedding-3-small"}
    };

    cpr::Response r = cpr::Post(
        cpr::Url{url},
        cpr::Body{data.dump()},
        cpr::Bearer{api_key},
        cpr::Header{{"Content-Type", "application/json"}}
    );
    if (r.status_code != 200) {
        throw std::runtime_error("Bad status: " + std::to_string(r.status_code));
    }
    json response = json::parse(r.text);

    std::vector<std::vector<float>> embeddings;
    for (auto& v: response["data"]) {
        embeddings.emplace_back(v["embedding"]);
    }
    return embeddings;
}

int main() {
    char *api_key = std::getenv("OPENAI_API_KEY");
    if (!api_key) {
        std::cout << "Set OPENAI_API_KEY" << std::endl;
        return 1;
    }

    pqxx::connection conn("dbname=pgvector_example");

    pqxx::nontransaction tx(conn);
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");
    tx.exec("DROP TABLE IF EXISTS documents");
    tx.exec("CREATE TABLE documents (id bigserial PRIMARY KEY, content text, embedding vector(1536))");

    std::vector<std::string> input = {
        "The dog is barking",
        "The cat is purring",
        "The bear is growling"
    };
    auto embeddings = embed(input, api_key);
    for (size_t i = 0; i < input.size(); i++) {
        tx.exec("INSERT INTO documents (content, embedding) VALUES ($1, $2)", pqxx::params{input[i], pgvector::Vector(embeddings[i])});
    }

    std::string query = "forest";
    auto query_embedding = embed({query}, api_key)[0];
    pqxx::result result = tx.exec("SELECT content FROM documents ORDER BY embedding <=> $1 LIMIT 5", pqxx::params{pgvector::Vector(query_embedding)});
    for (const auto& row : result) {
        std::cout << row[0].as<std::string>() << std::endl;
    }

    return 0;
}
