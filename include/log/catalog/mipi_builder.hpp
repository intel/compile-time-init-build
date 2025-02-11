#pragma once

#include <log/catalog/catalog.hpp>
#include <log/catalog/mipi_messages.hpp>

#include <stdx/compiler.hpp>
#include <stdx/utility.hpp>

#include <array>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <utility>

namespace logging::mipi {
template <typename> struct builder;

template <> struct builder<defn::short32_msg_t> {
    template <auto Level, std::same_as<std::uint32_t>... Ts>
    ALWAYS_INLINE static auto build(string_id id, module_id, Ts...) {
        using namespace msg;
        return owning<defn::short32_msg_t>{"payload"_field = id};
    }
};

template <typename Storage> struct catalog_builder {
    template <auto Level, std::same_as<std::uint32_t>... Ts>
    ALWAYS_INLINE static auto build(string_id id, module_id m, Ts... msg_data) {
        using namespace msg;
        defn::catalog_msg_t::owner_t<Storage> message{"severity"_field = Level,
                                                      "module_id"_field = m};

        constexpr auto header_size =
            defn::catalog_msg_t::size<typename Storage::value_type>::value;
        constexpr auto copy_arg = [](std::uint32_t arg, auto &dest) {
            std::memcpy(dest, &arg, sizeof(std::uint32_t));
            dest += sizeof(std::uint32_t);
        };
        auto dest = &message.data()[header_size];
        copy_arg(stdx::to_le(id), dest);
        (copy_arg(stdx::to_le(msg_data), dest), ...);

        return message;
    }
};

template <typename Storage>
    requires std::same_as<typename Storage::value_type, std::uint32_t>
struct catalog_builder<Storage> {
    template <auto Level, std::same_as<std::uint32_t>... Ts>
    ALWAYS_INLINE static auto build(string_id id, module_id m, Ts... msg_data) {
        using namespace msg;
        defn::catalog_msg_t::owner_t<Storage> message{"severity"_field = Level,
                                                      "module_id"_field = m};

        constexpr auto header_size =
            defn::catalog_msg_t::size<std::uint32_t>::value;
        auto dest = &message.data()[header_size];
        *dest++ = stdx::to_le(id);
        ((*dest++ = stdx::to_le(msg_data)), ...);

        return message;
    }
};

template <> struct builder<defn::catalog_msg_t> {
    template <auto Level, std::same_as<std::uint32_t>... Ts>
    ALWAYS_INLINE static auto build(string_id id, module_id m, Ts... msg_data) {
        using namespace msg;
        if constexpr (sizeof...(Ts) <= 2u) {
            constexpr auto header_size =
                defn::catalog_msg_t::size<std::uint32_t>::value;
            constexpr auto payload_len = 1 + sizeof...(Ts);
            using storage_t =
                std::array<std::uint32_t, header_size + payload_len>;
            return catalog_builder<storage_t>{}.template build<Level>(
                id, m, msg_data...);
        } else {
            constexpr auto header_size =
                defn::catalog_msg_t::size<std::uint8_t>::value;
            constexpr auto payload_len = (sizeof(id) + ... + sizeof(Ts));
            using storage_t =
                std::array<std::uint8_t, header_size + payload_len>;
            return catalog_builder<storage_t>{}.template build<Level>(
                id, m, msg_data...);
        }
    }
};

struct default_builder {
    template <auto Level, std::same_as<std::uint32_t>... Ts>
    ALWAYS_INLINE static auto build(string_id id, module_id m, Ts... msg_data) {
        if constexpr (sizeof...(Ts) == 0u) {
            return builder<defn::short32_msg_t>{}.template build<Level>(
                id, m, msg_data...);
        } else {
            return builder<defn::catalog_msg_t>{}.template build<Level>(
                id, m, msg_data...);
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
