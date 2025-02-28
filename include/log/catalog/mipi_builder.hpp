#pragma once

#include <log/catalog/catalog.hpp>
#include <log/catalog/mipi_messages.hpp>

#include <stdx/compiler.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <array>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <utility>

namespace logging::mipi {
template <typename T>
concept signed_packable =
    std::signed_integral<T> and sizeof(T) <= sizeof(std::int64_t);

template <typename T>
concept unsigned_packable =
    std::unsigned_integral<T> and sizeof(T) <= sizeof(std::int64_t);

template <typename T>
concept enum_packable = std::is_enum_v<T> and sizeof(T) <= sizeof(std::int32_t);

template <typename T>
concept packable =
    signed_packable<T> or unsigned_packable<T> or enum_packable<T>;

template <typename T> struct pack_as;

template <signed_packable T> struct pack_as<T> {
    using type =
        // NOLINTNEXTLINE(google-runtime-int)
        stdx::conditional_t<sizeof(T) <= sizeof(std::int32_t), int, long long>;
};

template <unsigned_packable T> struct pack_as<T> {
    using type = stdx::conditional_t<sizeof(T) <= sizeof(std::int32_t),
                                     // NOLINTNEXTLINE(google-runtime-int)
                                     unsigned int, unsigned long long>;
};

template <enum_packable T>
struct pack_as<T> : pack_as<stdx::underlying_type_t<T>> {};

template <packable T> using pack_as_t = typename pack_as<T>::type;

template <typename> struct builder;

template <> struct builder<defn::short32_msg_t> {
    template <auto Level> static auto build(string_id id, module_id) {
        using namespace msg;
        return owning<defn::short32_msg_t>{"payload"_field = id};
    }
};

template <typename Storage> struct catalog_builder {
    template <auto Level, packable... Ts>
    static auto build(string_id id, module_id m, Ts... args) {
        using namespace msg;
        defn::catalog_msg_t::owner_t<Storage> message{"severity"_field = Level,
                                                      "module_id"_field = m};

        using V = typename Storage::value_type;
        constexpr auto header_size = defn::catalog_msg_t::size<V>::value;

        auto const pack_arg = []<typename T>(V *p, T arg) -> V * {
            auto const packed = stdx::to_le(stdx::as_unsigned(
                static_cast<pack_as_t<T>>(stdx::to_underlying(arg))));
            std::memcpy(p, &packed, sizeof(packed));
            return p + stdx::sized8{sizeof(packed)}.in<V>();
        };

        auto dest = &message.data()[header_size];
        dest = pack_arg(dest, stdx::to_le(id));
        ((dest = pack_arg(dest, args)), ...);

        return message;
    }
};

template <> struct builder<defn::catalog_msg_t> {
    template <auto Level, typename... Ts>
    static auto build(string_id id, module_id m, Ts... args) {
        using namespace msg;
        if constexpr ((0 + ... + sizeof(Ts)) <= sizeof(std::uint32_t) * 2) {
            constexpr auto header_size =
                defn::catalog_msg_t::size<std::uint32_t>::value;
            constexpr auto payload_size =
                stdx::sized8{(sizeof(id) + ... + sizeof(pack_as_t<Ts>))}
                    .in<std::uint32_t>();
            using storage_t =
                std::array<std::uint32_t, header_size + payload_size>;
            return catalog_builder<storage_t>{}.template build<Level>(id, m,
                                                                      args...);
        } else {
            constexpr auto header_size =
                defn::catalog_msg_t::size<std::uint8_t>::value;
            constexpr auto payload_size = (sizeof(id) + ... + sizeof(Ts));
            using storage_t =
                std::array<std::uint8_t, header_size + payload_size>;
            return catalog_builder<storage_t>{}.template build<Level>(id, m,
                                                                      args...);
        }
    }
};

struct default_builder {
    template <auto Level, packable... Ts>
    static auto build(string_id id, module_id m, Ts... args) {
        if constexpr (sizeof...(Ts) == 0u) {
            return builder<defn::short32_msg_t>{}.template build<Level>(id, m);
        } else {
            return builder<defn::catalog_msg_t>{}.template build<Level>(
                id, m, args...);
        }
    }
};

[[maybe_unused]] constexpr inline struct get_builder_t {
    template <typename T>
        requires true
    CONSTEVAL auto operator()(T &&t) const noexcept(
        noexcept(std::forward<T>(t).query(std::declval<get_builder_t>())))
        -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const {
        return stdx::ct<default_builder{}>();
    }
} get_builder;
} // namespace logging::mipi
