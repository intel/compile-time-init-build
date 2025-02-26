#pragma once
#include <lookup/detail/select.hpp>
#include <lookup/input.hpp>
#include <lookup/strategy_failed.hpp>

#include <stdx/compiler.hpp>

#include <cstddef>
#include <iterator>

namespace lookup {
template <std::size_t MaxSize> struct linear_search_lookup {
  private:
    template <typename Input> struct impl : Input {
        using key_type = typename Input::key_type;
        using value_type = typename Input::value_type;

        [[nodiscard]] constexpr auto operator[](key_type key) const
            -> value_type {
            value_type result = this->default_value;
            for (auto [k, v] : this->entries) {
                result = detail::select(key, k, v, result);
            }
            return result;
        }
    };

  public:
    [[nodiscard]] CONSTEVAL static auto make(compile_time auto i) {
        if constexpr (constexpr auto input = i(); input.size <= MaxSize) {
            return impl<decltype(input)>{input};
        } else {
            return strategy_failed_t{};
        }
    }
};
} // namespace lookup
