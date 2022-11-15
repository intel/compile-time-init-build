#pragma once

#include <utility>

namespace cib::traits {
template <typename BuilderMeta> struct builder {
    using type = decltype(std::declval<BuilderMeta>().builder());
};

template <typename BuilderMeta>
using builder_t = typename builder<BuilderMeta>::type;

template <typename BuilderMeta> constexpr builder_t<BuilderMeta> builder_v = {};

template <typename BuilderMeta> struct interface {
    using type = decltype(std::declval<BuilderMeta>().interface());
};

template <typename BuilderMeta>
using interface_t = typename interface<BuilderMeta>::type;
} // namespace cib::traits
