# pgvector-cpp

[pgvector](https://github.com/pgvector/pgvector) support for C++

Supports [libpqxx](https://github.com/jtv/libpqxx)

[![Build Status](https://github.com/pgvector/pgvector-cpp/actions/workflows/build.yml/badge.svg)](https://github.com/pgvector/pgvector-cpp/actions)

## Installation

Add [the headers](https://github.com/pgvector/pgvector-cpp/tree/v0.2.1/include) to your project (supports C++17 and greater).

There is also support for CMake and FetchContent:

```cmake
include(FetchContent)

FetchContent_Declare(pgvector GIT_REPOSITORY https://github.com/pgvector/pgvector-cpp.git GIT_TAG v0.2.1)
FetchContent_MakeAvailable(pgvector)

target_link_libraries(app PRIVATE pgvector::pgvector)
```

## Getting Started

Follow the instructions for your database library:

- [libpqxx](#libpqxx)

Or check out some examples:

- [Embeddings](https://github.com/pgvector/pgvector-cpp/blob/master/examples/openai/example.cpp) with OpenAI
- [Binary embeddings](https://github.com/pgvector/pgvector-cpp/blob/master/examples/cohere/example.cpp) with Cohere
- [Hybrid search](https://github.com/pgvector/pgvector-cpp/blob/master/examples/hybrid/example.cpp) with llama.cpp (Reciprocal Rank Fusion)
- [Sparse search](https://github.com/pgvector/pgvector-cpp/blob/master/examples/sparse/example.cpp) with Text Embeddings Inference
- [Morgan fingerprints](https://github.com/pgvector/pgvector-cpp/blob/master/examples/rdkit/example.cpp) with RDKit
- [Recommendations](https://github.com/pgvector/pgvector-cpp/blob/master/examples/disco/example.cpp) with Disco
- [Horizontal scaling](https://github.com/pgvector/pgvector-cpp/blob/master/examples/citus/example.cpp) with Citus
- [Bulk loading](https://github.com/pgvector/pgvector-cpp/blob/master/examples/loading/example.cpp) with `COPY`

## libpqxx

Include the header

```cpp
#include <pgvector/pqxx.hpp>
```

Enable the extension

```cpp
tx.exec("CREATE EXTENSION IF NOT EXISTS vector");
```

Create a table

```cpp
tx.exec("CREATE TABLE items (id bigserial PRIMARY KEY, embedding vector(3))");
```

Insert a vector

```cpp
auto embedding = pgvector::Vector({1, 2, 3});
tx.exec("INSERT INTO items (embedding) VALUES ($1)", {embedding});
```

Get the nearest neighbors

```cpp
pqxx::result r = tx.exec("SELECT * FROM items ORDER BY embedding <-> $1 LIMIT 5", {embedding});
```

Retrieve a vector

```cpp
auto row = tx.exec("SELECT embedding FROM items LIMIT 1").one_row();
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
g++ -std=c++17 -Wall -Wextra -Wno-unknown-attributes -Werror -o test/main test/main.cpp test/pqxx_test.cpp -lpqxx -lpq
test/main
```

To run an example:

```sh
cd examples/loading
createdb pgvector_example
cmake -S . -B build
cmake --build build
build/example
```
