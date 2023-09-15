#pragma once
#include <lookup/detail/select.hpp>
#include <lookup/entry.hpp>
#include <lookup/input.hpp>
#include <lookup/strategy_failed.hpp>

#include <stdx/bit.hpp>
#include <stdx/compiler.hpp>

#include <array>
#include <cstddef>

namespace lookup {
// TODO: max load factor is unused?
template <int MaxLoadFactor, std::size_t SearchLen, typename HashFunc>
struct fast_hash_lookup {
  private:
    template <typename Input> struct impl {
        using key_type = typename Input::key_type;
        using value_type = typename Input::value_type;

        // fixme: increase size if needed
        constexpr static auto storage_size =
            static_cast<std::size_t>(2u << stdx::bit_width(Input::size()));

        value_type default_value;
        std::array<entry<key_type, value_type>, storage_size + SearchLen>
            storage;

        [[nodiscard]] constexpr auto index(key_type key) const -> std::size_t {
            auto const hash_value = HashFunc::calc(key);
            return static_cast<std::size_t>(hash_value) % storage_size;
        }

        CONSTEVAL explicit impl(auto const &input)
            : default_value{input.default_value} {
            storage.fill(entry{key_type{}, default_value});

            for (auto const &e : input.entries) {
                auto const i = index(e.key_);
                for (auto offset = std::size_t{}; offset <= SearchLen;
                     ++offset) {
                    if (storage[i + offset].value_ == default_value) {
                        storage[i + offset] = e;
                        break;
                    }
                }
            }
        }

        [[nodiscard]] constexpr auto operator[](key_type const key) const
            -> value_type {
            auto const idx = index(key);

            if constexpr (SearchLen > 0) {
                auto result = default_value;
                for (auto j = std::size_t{}; j < SearchLen; ++j) {
                    auto const [k, v] = storage[idx + j];
                    result = detail::select(key, k, v, result);
                }
                return result;
            } else {
                auto const [k, v] = storage[idx];
                return detail::select(key, k, v, default_value);
            }
        }
    };

  public:
    [[nodiscard]] CONSTEVAL static auto make(compile_time auto i) {
        constexpr auto input = i();
        constexpr auto candidate = impl<decltype(input)>(input);

        constexpr bool candidate_is_valid = [&]() {
            for (auto const [k, v] : input.entries) {
                if (candidate[k] != v) {
                    return false;
                }
            }

            return true;
        }();

        if constexpr (candidate_is_valid) {
            return candidate;
        } else {
            return strategy_failed_t{};
        }
    }
};
} // namespace lookup
