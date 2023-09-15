#pragma once

#define CX_VALUE(...)                                                          \
    [] {                                                                       \
        struct {                                                               \
            CONSTEVAL auto operator()() const noexcept { return __VA_ARGS__; } \
            using cx_value_t [[maybe_unused]] = void;                          \
        } val;                                                                 \
        return val;                                                            \
    }()
