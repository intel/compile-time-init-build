#pragma once

#include <log/log.hpp>

#include <type_traits>

namespace flow {
struct default_log_spec {
    using flavor = logging::default_flavor_t;
    constexpr static auto level = logging::level::TRACE;
};
template <stdx::ct_string, typename...>
constexpr auto log_spec = default_log_spec{};

template <typename T, typename... DummyArgs>
    requires(sizeof...(DummyArgs) == 0)
constexpr static auto get_log_spec() {
    if constexpr (std::is_same_v<decltype(log_spec<T::ct_name, DummyArgs...>),
                                 default_log_spec const>) {
        return log_spec<"default", DummyArgs...>;
    } else {
        return log_spec<T::ct_name, DummyArgs...>;
    }
}

template <typename T, typename... DummyArgs, typename... Args>
auto log_with_spec(Args &&...args) {
    using log_spec_t = decltype(get_log_spec<T, DummyArgs...>());
    logging::log<typename log_spec_t::flavor, log_spec_t::level,
                 cib_log_module_id_t>(std::forward<Args>(args)...);
}
} // namespace flow
