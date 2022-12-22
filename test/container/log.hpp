#pragma once

#include <log/log.hpp>

struct test_log_config : logging::null::config {
    struct exception {};

    struct [[nodiscard]] prevent_throws {
        prevent_throws() { throws = false; }
        prevent_throws(prevent_throws &&) = delete;
        auto operator=(prevent_throws &&) -> prevent_throws & = delete;
        ~prevent_throws() { throws = true; }
    };

    static auto terminate() -> void {
        if (throws) {
            throw exception{};
        }
    }

  private:
    static inline bool throws = true;
};

template <> inline auto logging::config<> = test_log_config{};
