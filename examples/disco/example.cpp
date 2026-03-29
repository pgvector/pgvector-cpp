#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include <disco.hpp>
#include <pgvector/pqxx.hpp>
#include <pqxx/pqxx>

std::string convert_to_utf8(const std::string& str) {
    std::stringstream buf;
    for (const unsigned char v : str) {
        if (v < 128) {
            buf << v;
        } else {
            buf << static_cast<unsigned char>(195) << static_cast<unsigned char>(v - 64);
        }
    }
    return buf.str();
}

disco::Dataset<int, std::string> load_movielens(const std::string& path) {
    std::string line;

    // read movies
    std::unordered_map<std::string, std::string> movies;
    std::ifstream movies_file(path + "/u.item");
    if (!movies_file.is_open()) {
        throw std::runtime_error{"Could not open file"};
    }
    while (std::getline(movies_file, line)) {
        std::string::size_type n = line.find('|');
        std::string::size_type n2 = line.find('|', n + 1);
        movies.emplace(line.substr(0, n), convert_to_utf8(line.substr(n + 1, n2 - n - 1)));
    }

    // read ratings and create dataset
    disco::Dataset<int, std::string> data;
    std::ifstream ratings_file(path + "/u.data");
    if (!ratings_file.is_open()) {
        throw std::runtime_error{"Could not open file"};
    }
    while (std::getline(ratings_file, line)) {
        std::string::size_type n = line.find('\t');
        std::string::size_type n2 = line.find('\t', n + 1);
        std::string::size_type n3 = line.find('\t', n2 + 1);
        data.push(
            std::stoi(line.substr(0, n)),
            movies.at(line.substr(n + 1, n2 - n - 1)),
            std::stof(line.substr(n2 + 1, n3 - n2 - 1))
        );
    }

    return data;
}

int main() {
    // https://grouplens.org/datasets/movielens/100k/
    char* movielens_path = std::getenv("MOVIELENS_100K_PATH");
    if (!movielens_path) {
        std::cout << "Set MOVIELENS_100K_PATH" << std::endl;
        return 1;
    }

    pqxx::connection conn{"dbname=pgvector_example"};

    pqxx::nontransaction tx{conn};
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");
    tx.exec("DROP TABLE IF EXISTS users");
    tx.exec("DROP TABLE IF EXISTS movies");
    tx.exec("CREATE TABLE users (id integer PRIMARY KEY, factors vector(20))");
    tx.exec("CREATE TABLE movies (name text PRIMARY KEY, factors vector(20))");

    disco::Dataset<int, std::string> data = load_movielens(movielens_path);
    auto recommender = disco::Recommender<int, std::string>::fit_explicit(data, {.factors = 20});

    for (const auto& user_id : recommender.user_ids()) {
        pgvector::Vector factors{*recommender.user_factors(user_id)};
        tx.exec("INSERT INTO users (id, factors) VALUES ($1, $2)", pqxx::params{user_id, factors});
    }

    for (const auto& item_id : recommender.item_ids()) {
        pgvector::Vector factors{*recommender.item_factors(item_id)};
        tx.exec(
            "INSERT INTO movies (name, factors) VALUES ($1, $2)", pqxx::params{item_id, factors}
        );
    }

    std::string movie{"Star Wars (1977)"};
    std::cout << "Item-based recommendations for " << movie << std::endl;
    pqxx::result result = tx.exec(
        "SELECT name FROM movies WHERE name != $1 ORDER BY factors <=> (SELECT factors FROM movies WHERE name = $1) LIMIT 5",
        pqxx::params{movie}
    );
    for (const auto& row : result) {
        std::cout << "- " << row[0].as<std::string>() << std::endl;
    }

    int user_id = 123;
    std::cout << std::endl << "User-based recommendations for " << user_id << std::endl;
    result = tx.exec(
        "SELECT name FROM movies ORDER BY factors <#> (SELECT factors FROM users WHERE id = $1) LIMIT 5",
        pqxx::params{user_id}
    );
    for (const auto& row : result) {
        std::cout << "- " << row[0].as<std::string>() << std::endl;
    }

    return 0;
}
