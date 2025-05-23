#pragma once

#include <stdx/type_traits.hpp>

#include <concepts>
#include <cstdint>
#include <type_traits>

template <typename> struct encode_32;
template <typename> struct encode_64;
template <typename> struct encode_u32;
template <typename> struct encode_u64;

namespace logging {
template <typename T>
concept signed_packable = std::signed_integral<stdx::underlying_type_t<T>> and
                          sizeof(T) <= sizeof(std::int64_t);

template <typename T>
concept unsigned_packable =
    std::unsigned_integral<stdx::underlying_type_t<T>> and
    sizeof(T) <= sizeof(std::int64_t);

template <typename T>
concept float_packable = std::floating_point<stdx::underlying_type_t<T>> and
                         sizeof(T) <= sizeof(std::int64_t);

template <typename T>
concept packable =
    signed_packable<T> or unsigned_packable<T> or float_packable<T>;

template <typename T> struct encoding;

template <signed_packable T> struct encoding<T> {
    using encode_t = stdx::conditional_t<sizeof(T) <= sizeof(std::int32_t),
                                         encode_32<T>, encode_64<T>>;
    using pack_t = stdx::conditional_t<sizeof(T) <= sizeof(std::int32_t),
                                       std::int32_t, std::int64_t>;
};

template <unsigned_packable T> struct encoding<T> {
    using encode_t = stdx::conditional_t<sizeof(T) <= sizeof(std::uint32_t),
                                         encode_u32<T>, encode_u64<T>>;
    using pack_t = stdx::conditional_t<sizeof(T) <= sizeof(std::uint32_t),
                                       std::uint32_t, std::uint64_t>;
};

template <float_packable T> struct encoding<T> {
    using encode_t = stdx::conditional_t<sizeof(T) <= sizeof(std::uint32_t),
                                         encode_u32<T>, encode_u64<T>>;
    using pack_t = stdx::conditional_t<sizeof(T) <= sizeof(std::uint32_t),
                                       std::uint32_t, std::uint64_t>;
};

struct default_arg_packer {
    template <packable T> using pack_as_t = typename encoding<T>::pack_t;
    template <packable T> using encode_as_t = typename encoding<T>::encode_t;
};
} // namespace logging
