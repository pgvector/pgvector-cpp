#pragma once

#include <functional>
#include <optional>
#include <source_location>
#include <sstream>
#include <string_view>

template<typename T, typename U>
void assert_equal(const T& left, const U& right, const std::source_location& loc = std::source_location::current()) {
    if (left != right) {
        std::ostringstream message;
        message << left << " != " << right;
        message << " in " << loc.function_name() << " " << loc.file_name() << ":" << loc.line();
        throw std::runtime_error{message.str()};
    }
}

template<typename T>
void assert_exception(std::function<void(void)> code, std::optional<std::string_view> message = std::nullopt) {
    std::optional<T> exception;
    try {
        code();
    } catch (const T& e) {
        exception = e;
    }
    assert_equal(exception.has_value(), true);
    if (message) {
        assert_equal(std::string_view{exception.value().what()}, message.value());
    }
}
