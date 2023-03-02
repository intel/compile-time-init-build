#pragma once

#include <cib/detail/compiler.hpp>
#include <cib/tuple.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <string_view>

namespace cib::detail {
struct typename_map_entry {
    std::string_view name{};
    std::size_t index{};
    std::size_t src{};

  private:
    [[nodiscard]] constexpr friend auto
    operator<=>(typename_map_entry const &lhs, typename_map_entry const &rhs) {
        return lhs.name <=> rhs.name;
    }

    [[nodiscard]] constexpr friend auto
    operator==(typename_map_entry const &lhs, typename_map_entry const &rhs)
        -> bool {
        return lhs.name == rhs.name;
    }
};

template <typename Tag> CIB_CONSTEVAL static auto name() -> std::string_view {
#if defined(__clang__)
    constexpr std::string_view function_name = __PRETTY_FUNCTION__;
    constexpr auto lhs = 44;
    constexpr auto rhs = function_name.size() - 2;

#elif defined(__GNUC__) || defined(__GNUG__)
    constexpr std::string_view function_name = __PRETTY_FUNCTION__;
    constexpr auto lhs = 59;
    constexpr auto rhs = function_name.size() - 51;

#elif defined(_MSC_VER)
    constexpr std::string_view function_name = __FUNCSIG__;
    constexpr auto lhs = 0;
    constexpr auto rhs = function_name.size();
#else
    static_assert(false, "Unknown compiler, can't build cib::tuple name.");
#endif

    return function_name.substr(lhs, rhs - lhs + 1);
}

template <template <typename> typename MetaFunc, typename... Types>
CIB_CONSTEVAL static auto create_type_names([[maybe_unused]] std::size_t src) {
    auto i = std::size_t{};
    std::array<typename_map_entry, sizeof...(Types)> names = {
        typename_map_entry{name<MetaFunc<Types>>(), i++, src}...};
    std::sort(std::begin(names), std::end(names));
    return names;
}
} // namespace cib::detail
