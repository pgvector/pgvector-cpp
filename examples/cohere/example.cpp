#include <bitset>
#include <cstdint>
#include <iostream>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <pgvector/pqxx.hpp>
#include <pqxx/pqxx>

using json = nlohmann::json;

// https://docs.cohere.com/reference/embed
std::vector<std::string> fetch_embeddings(const std::vector<std::string>& texts, const std::string& input_type, char *api_key) {
    std::string url = "https://api.cohere.com/v1/embed";
    json data = {
        {"texts", texts},
        {"model", "embed-english-v3.0"},
        {"input_type", input_type},
        {"embedding_types", {"ubinary"}}
    };

    cpr::Response r = cpr::Post(
        cpr::Url{url},
        cpr::Body{data.dump()},
        cpr::Bearer{api_key},
        cpr::Header{{"Content-Type", "application/json"}}
    );
    json response = json::parse(r.text);

    std::vector<std::string> embeddings;
    for (auto& v: response["embeddings"]["ubinary"]) {
        std::stringstream buf;
        for (uint8_t c : v) {
            std::bitset<8> b{c};
            buf << b.to_string();
        }
        embeddings.emplace_back(buf.str());
    }
    return embeddings;
}

int main() {
    char *api_key = std::getenv("CO_API_KEY");
    if (!api_key) {
        std::cout << "Set CO_API_KEY" << std::endl;
        return 1;
    }

    pqxx::connection conn("dbname=pgvector_example");

    pqxx::work tx(conn);
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");
    tx.exec("DROP TABLE IF EXISTS documents");
    tx.exec("CREATE TABLE documents (id bigserial PRIMARY KEY, content text, embedding bit(1024))");
    tx.commit();

    std::vector<std::string> input = {
        "The dog is barking",
        "The cat is purring",
        "The bear is growling"
    };
    auto embeddings = fetch_embeddings(input, "search_document", api_key);

    for (size_t i = 0; i < input.size(); i++) {
        tx.exec("INSERT INTO documents (content, embedding) VALUES ($1, $2)", pqxx::params{input[i], embeddings[i]});
    }
    tx.commit();

    std::string query = "forest";
    auto query_embedding = fetch_embeddings({query}, "search_query", api_key)[0];
    pqxx::result result = tx.exec("SELECT content FROM documents ORDER BY embedding <~> $1 LIMIT 5", pqxx::params{query_embedding});
    for (const auto& row : result) {
        std::cout << row[0].as<std::string>() << std::endl;
    }

    return 0;
}
