#pragma once

#include <cassert>
#include <functional>
#include <optional>
#include <string_view>

template<typename T>
void assert_exception(std::function<void(void)> code, std::optional<std::string_view> message = std::nullopt) {
    bool exception = false;
    try {
        code();
    } catch (const T& e) {
        exception = true;
        if (message) {
            assert(std::string_view(e.what()) == *message);
        }
    }
    assert(exception);
}
