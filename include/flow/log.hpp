#pragma once

#include <log/env.hpp>
#include <log/log.hpp>

#include <stdx/ct_string.hpp>

#include <type_traits>

namespace flow {
using default_log_env =
    stdx::make_env_t<logging::get_level, logging::level::TRACE>;
template <stdx::ct_string, typename...>
constexpr auto log_env = default_log_env{};

template <stdx::ct_string Name> struct log_env_id_t {
    constexpr static auto ct_name = Name;
};

template <typename T, typename Fallback = log_env_id_t<"default">,
          typename... DummyArgs>
    requires(sizeof...(DummyArgs) == 0)
constexpr static auto get_log_env() {
    using log_env_t = decltype(log_env<T::ct_name, DummyArgs...>);
    if constexpr (std::is_same_v<log_env_t, default_log_env const>) {
        if constexpr (Fallback::ct_name == stdx::ct_string{"default"}) {
            return log_env<Fallback::ct_name, DummyArgs...>;
        } else {
            return get_log_env<Fallback>();
        }
    } else {
        return log_env_t{};
    }
}
} // namespace flow
