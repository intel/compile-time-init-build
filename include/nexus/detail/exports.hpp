#pragma once

#include <nexus/detail/config_item.hpp>
#include <nexus/detail/extend.hpp>

#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

namespace cib::detail {
template <typename ServiceT, typename BuilderT> struct service_entry {
    using Service = ServiceT;
    BuilderT builder;
};

template <typename... Services> struct exports : public detail::config_item {
    [[nodiscard]] constexpr auto extends_tuple() const
        -> stdx::tuple<type_extend<Services>...> {
        return {type_extend<Services>{}...};
    }

    [[nodiscard]] constexpr static auto get_exports()
        -> stdx::type_list<Services...> {
        return {};
    }
};
} // namespace cib::detail
