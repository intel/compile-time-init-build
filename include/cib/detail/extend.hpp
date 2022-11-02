#include <cib/detail/config_item.hpp>
#include <cib/tuple.hpp>

#include <utility>

#ifndef COMPILE_TIME_INIT_BUILD_EXTEND_HPP
#define COMPILE_TIME_INIT_BUILD_EXTEND_HPP

namespace cib::detail {
template <typename ServiceType, typename... Args>
struct extend : public config_item {
    using service_type = ServiceType;
    constexpr static auto builder = cib::traits::builder_v<service_type>;
    cib::tuple<Args...> args_tuple;

    CIB_CONSTEVAL explicit extend(Args const &...args)
        : args_tuple{cib::make_tuple(args...)} {
        // pass
    }

    template <typename... InitArgs>
    [[nodiscard]] CIB_CONSTEVAL auto extends_tuple(InitArgs const &...) const {
        return cib::make_tuple(*this);
    }
};
} // namespace cib::detail

#endif // COMPILE_TIME_INIT_BUILD_EXTEND_HPP
