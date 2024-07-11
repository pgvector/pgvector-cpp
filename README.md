# pgvector-cpp

[pgvector](https://github.com/pgvector/pgvector) support for C++

Supports [libpqxx](https://github.com/jtv/libpqxx)

[![Build Status](https://github.com/pgvector/pgvector-cpp/actions/workflows/build.yml/badge.svg)](https://github.com/pgvector/pgvector-cpp/actions)

## Installation

Run:

```sh
git clone --branch v0.2.0 https://github.com/pgvector/pgvector-cpp.git
cd pgvector-cpp
cmake -S . -B build
cmake --build build
cmake --install build # may need sudo
```

And follow the instructions for your database library:

- [libpqxx](#libpqxx)

## libpqxx

Include the header

```cpp
#include <pgvector/pqxx.hpp>
```

Enable the extension

```cpp
tx.exec0("CREATE EXTENSION IF NOT EXISTS vector");
```

Create a table

```cpp
tx.exec0("CREATE TABLE items (id bigserial PRIMARY KEY, embedding vector(3))");
```

Insert a vector

```cpp
auto embedding = pgvector::Vector({1, 2, 3});
tx.exec_params("INSERT INTO items (embedding) VALUES ($1)", embedding);
```

Get the nearest neighbors

```cpp
pqxx::result R{tx.exec_params("SELECT * FROM items ORDER BY embedding <-> $1 LIMIT 5", embedding)};
```

Retrieve a vector

```cpp
auto row = tx.exec1("SELECT embedding FROM items LIMIT 1");
auto embedding = row[0].as<pgvector::Vector>();
```

Use `std::optional<pgvector::Vector>` if the value could be `NULL`

## Reference

Convert a vector to a `std::vector<float>`

```cpp
auto float_vec = static_cast<std::vector<float>>(embedding);
```

## History

View the [changelog](https://github.com/pgvector/pgvector-cpp/blob/master/CHANGELOG.md)

## Contributing

Everyone is encouraged to help improve this project. Here are a few ways you can help:

- [Report bugs](https://github.com/pgvector/pgvector-cpp/issues)
- Fix bugs and [submit pull requests](https://github.com/pgvector/pgvector-cpp/pulls)
- Write, clarify, or fix documentation
- Suggest or add new features

To get started with development:

```sh
git clone https://github.com/pgvector/pgvector-cpp.git
cd pgvector-cpp
createdb pgvector_cpp_test
g++ -std=c++17 -Wall -Wextra -Werror -o test/pqxx test/pqxx_test.cpp -lpqxx -lpq
test/pqxx
```
