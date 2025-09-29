#pragma once

#include <log/catalog/arguments.hpp>
#include <log/catalog/catalog.hpp>
#include <log/catalog/mipi_messages.hpp>

#include <stdx/bit.hpp>
#include <stdx/compiler.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

namespace logging::mipi {
template <typename T>
concept packer = std::integral<typename T::template pack_as_t<int>> and
                 requires { typename T::template encode_as_t<int>; };

template <typename, packer> struct builder;

template <packer P> struct builder<defn::short32_msg_t, P> {
    template <auto Level> static auto build(string_id id, module_id, unit_t) {
        using namespace msg;
        return owning<defn::short32_msg_t>{"payload"_field = id};
    }
};

template <typename Storage, packer P> struct catalog_builder {
    template <auto Level, packable... Ts>
    static auto build(string_id id, module_id m, unit_t u, Ts... args)
        -> defn::catalog_msg_t::owner_t<Storage> {
        using namespace msg;
        defn::catalog_msg_t::owner_t<Storage> message{
            "severity"_field = Level, "module_id"_field = m, "unit"_field = u};

        using V = typename Storage::value_type;
        constexpr auto header_size = defn::catalog_msg_t::size<V>::value;

        auto const pack_arg = []<typename T>(V *p, T arg) -> V * {
            typename P::template pack_as_t<T> converted{};
            if constexpr (sizeof(stdx::to_underlying(arg)) ==
                          sizeof(converted)) {
                converted = stdx::bit_cast<decltype(converted)>(
                    stdx::to_underlying(arg));
            } else {
                converted =
                    static_cast<decltype(converted)>(stdx::to_underlying(arg));
            }
            auto const packed = stdx::to_le(stdx::as_unsigned(converted));
            std::memcpy(p, &packed, sizeof(packed));
            return p + stdx::sized8{sizeof(packed)}.in<V>();
        };

        auto dest = &message.data()[header_size];
        dest = pack_arg(dest, stdx::to_le(id));
        ((dest = pack_arg(dest, args)), ...);

        return message;
    }
};

template <packer P> struct builder<defn::catalog_msg_t, P> {
    template <auto Level, typename... Ts>
    static auto build(string_id id, module_id m, unit_t u, Ts... args) {
        using namespace msg;
        constexpr auto payload_size =
            (sizeof(id) + ... + sizeof(typename P::template pack_as_t<Ts>));
        constexpr auto header_size =
            defn::catalog_msg_t::size<std::uint32_t>::value;
        using storage_t =
            std::array<std::uint32_t, header_size + stdx::sized8{payload_size}
                                                        .in<std::uint32_t>()>;
        return catalog_builder<storage_t, P>{}.template build<Level>(id, m, u,
                                                                     args...);
    }
};

template <packer P> struct builder<defn::compact32_build_msg_t, P> {
    template <auto Version> static auto build() {
        using namespace msg;
        return owning<defn::compact32_build_msg_t>{"build_id"_field = Version};
    }
};

template <packer P> struct builder<defn::compact64_build_msg_t, P> {
    template <auto Version> static auto build() {
        using namespace msg;
        return owning<defn::compact64_build_msg_t>{"build_id"_field = Version};
    }
};

template <packer P> struct builder<defn::normal_build_msg_t, P> {
    template <auto Version, stdx::ct_string S> static auto build() {
        using namespace msg;
        constexpr auto header_size =
            defn::normal_build_msg_t::size<std::uint8_t>::value;
        constexpr auto payload_len = S.size() + sizeof(std::uint64_t);
        using storage_t = std::array<std::uint8_t, header_size + payload_len>;

        defn::normal_build_msg_t::owner_t<storage_t> message{
            "payload_len"_field = payload_len};
        auto dest = &message.data()[header_size];

        auto const ver = stdx::to_le(static_cast<std::uint64_t>(Version));
        std::memcpy(dest, &ver, sizeof(std::uint64_t));
        dest += sizeof(std::uint64_t);
        std::copy_n(std::cbegin(S.value), S.size(), dest);
        return message;
    }
};

template <packer P = logging::default_arg_packer> struct default_builder {
    template <auto Level, packable... Ts>
    static auto build(string_id id, module_id m, unit_t unit, Ts... args) {
        if constexpr (sizeof...(Ts) == 0u) {
            return builder<defn::short32_msg_t, P>{}.template build<Level>(
                id, m, unit);
        } else {
            return builder<defn::catalog_msg_t, P>{}.template build<Level>(
                id, m, unit, args...);
        }
    }

    template <auto Version, stdx::ct_string S = ""> auto build_version() {
        using namespace msg;
        if constexpr (S.empty() and stdx::bit_width(Version) <= 22) {
            return builder<defn::compact32_build_msg_t, P>{}
                .template build<Version>();
        } else if constexpr (S.empty() and stdx::bit_width(Version) <= 54) {
            return builder<defn::compact64_build_msg_t, P>{}
                .template build<Version>();
        } else {
            return builder<defn::normal_build_msg_t, P>{}
                .template build<Version, S>();
        }
    }

    template <template <typename...> typename F, typename... Args>
    using convert_args = F<typename P::template encode_as_t<Args>...>;
};
} // namespace logging::mipi
