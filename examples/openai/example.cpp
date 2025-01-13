#include <cstdint>
#include <iostream>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <pgvector/pqxx.hpp>
#include <pqxx/pqxx>

using json = nlohmann::json;

// https://platform.openai.com/docs/guides/embeddings/how-to-get-embeddings
// input can be an array with 2048 elements
std::vector<std::vector<float>> fetch_embeddings(const std::vector<std::string>& input, char *api_key) {
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

    pqxx::work tx(conn);
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");
    tx.exec("DROP TABLE IF EXISTS documents");
    tx.exec("CREATE TABLE documents (id bigserial PRIMARY KEY, content text, embedding vector(1536))");
    tx.commit();

    std::vector<std::string> input = {
        "The dog is barking",
        "The cat is purring",
        "The bear is growling"
    };
    auto embeddings = fetch_embeddings(input, api_key);

    for (size_t i = 0; i < input.size(); i++) {
        tx.exec("INSERT INTO documents (content, embedding) VALUES ($1, $2)", {input[i], pgvector::Vector(embeddings[i])});
    }
    tx.commit();

    int64_t document_id = 1;
    pqxx::result result = tx.exec("SELECT content FROM documents WHERE id != $1 ORDER BY embedding <=> (SELECT embedding FROM documents WHERE id = $1) LIMIT 5", {document_id});
    for (auto const& row : result) {
        std::cout << row[0].c_str() << std::endl;
    }

    return 0;
}
