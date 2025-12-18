#pragma once

#include <nexus/detail/config_item.hpp>
#include <nexus/detail/extend.hpp>

#include <stdx/tuple.hpp>

namespace cib::detail {
template <typename ServiceT, typename BuilderT> struct service_entry {
    using Service = ServiceT;
    BuilderT builder;
};

template <typename... Services> struct exports : public detail::config_item {
    [[nodiscard]] constexpr auto extends_tuple() const
        -> stdx::tuple<extend<Services>...> {
        return {extend<Services>{}...};
    }

    [[nodiscard]] constexpr auto exports_tuple() const
        -> stdx::tuple<Services...> {
        return {};
    }
};
} // namespace cib::detail
