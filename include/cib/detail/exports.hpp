#include <cib/detail/builder_traits.hpp>
#include <cib/detail/compiler.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/extend.hpp>
#include <cib/detail/type_list.hpp>

#include <utility>

#ifndef COMPILE_TIME_INIT_BUILD_EXPORTS_HPP
#define COMPILE_TIME_INIT_BUILD_EXPORTS_HPP

namespace cib::detail {
template <typename ServiceT, typename BuilderT> struct service_entry {
    using Service = ServiceT;
    BuilderT builder;
};

template <typename... Services> struct exports : public detail::config_item {
    template <typename... InitArgs>
    [[nodiscard]] CIB_CONSTEXPR auto extends_tuple(InitArgs const &...) const {
        return cib::make_tuple(extend<Services>{}...);
    }
};
} // namespace cib::detail

#endif // COMPILE_TIME_INIT_BUILD_EXPORTS_HPP
