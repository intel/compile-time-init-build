#pragma once

#include <algorithm>
#include <bit>
#include <array>

namespace lookup {
    template<int MinLoadFactor>
    struct direct_array_lookup {
    private:
        template<typename InputValues>
        struct details {
            constexpr static auto key_lt = [](auto lhs, auto rhs){return lhs.key_ < rhs.key_;};
            constexpr static auto min_key = std::min_element(InputValues::entries.begin(), InputValues::entries.end(), key_lt)->key_;
            constexpr static auto max_key = std::max_element(InputValues::entries.begin(), InputValues::entries.end(), key_lt)->key_;
            constexpr static auto storage_size = 1 << std::bit_width(max_key - min_key);
            constexpr static auto num_keys = InputValues::entries.size();
            constexpr static auto load_factor = (num_keys * 100) / storage_size;
        };

        template<typename InputValues>
        struct impl {
        private:
            using key_type = typename InputValues::key_type;
            using value_type = typename InputValues::value_type;

            constexpr static auto default_value = InputValues::default_value;
            constexpr static auto min_key = details<InputValues>::min_key;

            std::array<value_type, details<InputValues>::storage_size> storage{};

        public:
            constexpr impl() {
                storage.fill(default_value);

                for (auto e : InputValues::entries) {
                    const auto index = e.key_ - min_key;
                    storage[index] = e.value_;
                }
            }

            [[nodiscard]] constexpr auto operator[](key_type key) const -> value_type {
                const auto index = key - min_key;
                const auto safe_index = index % storage.size();
                const auto stored_value = storage[safe_index];

                if (index == safe_index) {
                    return stored_value;
                } else {
                    return default_value;
                }
            }
        };

    public:
        template<typename InputValues>
        [[nodiscard]] consteval static auto make() {
            constexpr auto load_factor = details<InputValues>::load_factor;

            if constexpr (load_factor >= MinLoadFactor) {
                return impl<InputValues>();
            } else {
                return strategy_failed_t{};
            }
        }
    };
}
