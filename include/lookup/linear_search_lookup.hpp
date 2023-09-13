#pragma once

#include <lookup/detail/select.hpp>
#include <lookup/strategy_failed.hpp>

#include <stdx/compiler.hpp>

#include <cstddef>

namespace lookup {
template <std::size_t MaxSize> struct linear_search_lookup {
  private:
    template <typename InputValues> struct impl {
        using key_type = typename InputValues::key_type;
        using value_type = typename InputValues::value_type;

        [[nodiscard]] constexpr auto operator[](key_type key) const
            -> value_type {
            value_type result = InputValues::default_value;

            for (auto [k, v] : InputValues::entries) {
                result = detail::select(key, k, v, result);
            }

            return result;
        }
    };

  public:
    template <typename InputValues> [[nodiscard]] CONSTEVAL static auto make() {
        if constexpr (InputValues::entries.size() <= MaxSize) {
            return impl<InputValues>();
        } else {
            return strategy_failed_t{};
        }
    }
};
} // namespace lookup
