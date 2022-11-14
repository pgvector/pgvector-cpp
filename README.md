# pgvector-cpp

[pgvector](https://github.com/pgvector/pgvector) support for C++

Supports [libpqxx](https://github.com/jtv/libpqxx)

[![Build Status](https://github.com/pgvector/pgvector-cpp/workflows/build/badge.svg?branch=master)](https://github.com/pgvector/pgvector-cpp/actions)

## Getting Started

Run

```sh
git clone --branch v0.1.1 https://github.com/pgvector/pgvector-cpp.git
cd pgvector-cpp && mkdir build && cd build
cmake ..
make install # may need sudo
```

Include the header

```cpp
#include <pgvector/pqxx.hpp>
```

Insert a vector

```cpp
auto factors = pgvector::Vector({1, 2, 3});
W.exec_params("INSERT INTO items (factors) VALUES ($1)", factors);
```

Get the nearest neighbors

```cpp
pqxx::result R{W.exec_params("SELECT * FROM items ORDER BY factors <-> $1 LIMIT 5", factors)};
```

Retrieve a vector

```cpp
auto row = W.exec1("SELECT factors FROM items LIMIT 1");
auto factors = row[0].as<pgvector::Vector>();
```

Use `std::optional<pgvector::Vector>` if the value could be `NULL`

## Reference

Convert a vector to a `std::vector<float>`

```cpp
auto float_vec = static_cast<std::vector<float>>(factors);
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
