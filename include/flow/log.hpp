#pragma once

#include <log/log.hpp>

#include <stdx/ct_string.hpp>

#include <type_traits>

namespace flow {
struct default_log_spec {
    using flavor = logging::default_flavor_t;
    constexpr static auto level = logging::level::TRACE;
};
template <stdx::ct_string, typename...>
constexpr auto log_spec = default_log_spec{};

template <stdx::ct_string Name> struct log_spec_id_t {
    constexpr static auto ct_name = Name;
};

template <typename T, typename Fallback = log_spec_id_t<"default">,
          typename... DummyArgs>
    requires(sizeof...(DummyArgs) == 0)
constexpr static auto get_log_spec() {
    using log_spec_t = decltype(log_spec<T::ct_name, DummyArgs...>);
    if constexpr (std::is_same_v<log_spec_t, default_log_spec const>) {
        if constexpr (Fallback::ct_name == stdx::ct_string{"default"}) {
            return log_spec<Fallback::ct_name, DummyArgs...>;
        } else {
            return get_log_spec<Fallback>();
        }
    } else {
        return log_spec_t{};
    }
}
} // namespace flow
