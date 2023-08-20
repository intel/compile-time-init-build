#pragma once

#include <cib/detail/compiler.hpp>
#include <sc/detail/conversions.hpp>

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
    [[nodiscard]] constexpr friend auto operator<(typename_map_entry const &lhs,
                                                  typename_map_entry const &rhs)
        -> bool {
        return lhs.name < rhs.name;
    }

    [[nodiscard]] constexpr friend auto
    operator==(typename_map_entry const &lhs, typename_map_entry const &rhs)
        -> bool {
        return lhs.name == rhs.name;
    }
};

template <template <typename> typename MetaFunc, typename... Types>
CIB_CONSTEVAL static auto create_type_names([[maybe_unused]] std::size_t src) {
    auto i = std::size_t{};
    std::array<typename_map_entry, sizeof...(Types)> names = {
        typename_map_entry{sc::detail::type_as_string<MetaFunc<Types>>(), i++,
                           src}...};
    std::sort(std::begin(names), std::end(names));
    return names;
}
} // namespace cib::detail
