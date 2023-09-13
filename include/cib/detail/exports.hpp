#pragma once

#include <cib/detail/config_item.hpp>
#include <cib/detail/extend.hpp>

#include <stdx/tuple.hpp>

namespace cib::detail {
template <typename ServiceT, typename BuilderT> struct service_entry {
    using Service = ServiceT;
    BuilderT builder;
};

template <typename... Services> struct exports : public detail::config_item {
    template <typename... InitArgs>
    [[nodiscard]] constexpr auto extends_tuple(InitArgs const &...) const {
        return stdx::make_tuple(extend<Services>{}...);
    }

    template <typename... InitArgs>
    [[nodiscard]] constexpr auto exports_tuple(InitArgs const &...) const {
        return stdx::make_tuple(Services{}...);
    }
};
} // namespace cib::detail
