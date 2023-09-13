#pragma once

#include <cib/builder_meta.hpp>
#include <cib/detail/config_item.hpp>

#include <stdx/compiler.hpp>
#include <stdx/tuple.hpp>

namespace cib::detail {
template <typename ServiceType, typename... Args>
struct extend : public config_item {
    using service_type = ServiceType;
    constexpr static auto builder = cib::builder_t<service_type>{};
    stdx::tuple<Args...> args_tuple;

    CONSTEVAL explicit extend(Args const &...args) : args_tuple{args...} {}

    template <typename... InitArgs>
    [[nodiscard]] constexpr auto extends_tuple(InitArgs const &...) const {
        return stdx::make_tuple(*this);
    }
};
} // namespace cib::detail
