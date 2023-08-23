#pragma once

#include <lookup/detail/select.hpp>
#include <lookup/strategy_failed.hpp>

namespace lookup {
template <int MaxSize> struct linear_search_lookup {
  private:
    template <typename InputValues> struct impl {
        using key_type = typename InputValues::key_type;
        using value_type = typename InputValues::value_type;

        [[nodiscard]] constexpr auto operator[](key_type key) const
            -> value_type {
            value_type result = InputValues::default_value;

            for (auto e : InputValues::entries) {
                result = detail::select(key, e.key_, e.value_, result);
            }

            return result;
        }
    };

  public:
    template <typename InputValues> [[nodiscard]] consteval static auto make() {
        if constexpr (InputValues::entries.size() <= MaxSize) {
            return impl<InputValues>();
        } else {
            return strategy_failed_t{};
        }
    }
};
} // namespace lookup
