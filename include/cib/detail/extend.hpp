#pragma once

#include <cib/detail/builder_traits.hpp>
#include <cib/detail/compiler.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/tuple.hpp>

namespace cib::detail {
template <typename ServiceType, typename... Args>
struct extend : public config_item {
    using service_type = ServiceType;
    constexpr static auto builder = cib::traits::builder_v<service_type>;
    cib::tuple<Args...> args_tuple;

    CIB_CONSTEVAL explicit extend(Args const &...args) : args_tuple{args...} {}

    template <typename... InitArgs>
    [[nodiscard]] constexpr auto extends_tuple(InitArgs const &...) const {
        return cib::make_tuple(*this);
    }
};
} // namespace cib::detail
