#pragma once

#include <cstdint>

namespace sc {
template <typename...> struct args;
template <char...> struct string {};
template <typename, auto, auto> struct named_arg {};
template <typename...> struct named_args;
template <typename, auto, typename...> struct undefined;

template <typename> struct message {};
template <typename> struct module_string {};
} // namespace sc

using string_id = std::uint32_t;
using module_id = std::uint32_t;

template <typename> extern auto catalog() -> string_id;
template <typename> extern auto module() -> module_id;
