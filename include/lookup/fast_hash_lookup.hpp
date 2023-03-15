#pragma once

#include <lookup/entry.hpp>
#include <lookup/strategy_failed.hpp>

#include <bit>
#include <array>


namespace lookup {
    template<int MaxLoadFactor, int SearchLen, typename HashFunc>
    struct fast_hash_lookup {
    private:
        template<typename InputValues>
        struct impl {
            using key_type = typename InputValues::key_type;
            using value_type = typename InputValues::value_type;
            constexpr static auto default_value = InputValues::default_value;

            // fixme: increase size if needed
            constexpr static auto storage_size =
                2 << (std::bit_width(InputValues::entries.size()));

            std::array<entry<key_type, value_type>, storage_size + SearchLen> storage;

            [[nodiscard]] constexpr auto index(key_type key) const -> size_t {
                auto const hash_value = HashFunc::calc(key);
                return hash_value % storage_size;
            }

            consteval impl() {
                storage.fill(entry{key_type{}, default_value});

                for (auto const e : InputValues::entries) {
                    auto const i = index(e.key_);
                    for (auto offset = std::size_t{}; offset <= SearchLen; offset++) {
                        if (storage[i + offset].value_ == default_value) {
                            storage[i + offset] = e;
                            break;
                        }
                    }
                }
            }

            [[nodiscard]] constexpr auto operator[](key_type const key) const -> value_type {
                auto const i = index(key);
                value_type result = default_value;
                entry<key_type, value_type> e;

                if constexpr (SearchLen >= 1) {
                    e = storage[i + 1];
                    result = detail::select(key, e.key_, e.value_, result);
                }

                e = storage[i + 0];
                result = detail::select(key, e.key_, e.value_, result);

                //for (auto offset = std::size_t{}; offset <= 0; offset++) {
                //    entry const e = storage[i + offset];
                //    result = detail::select(key, e.key_, e.value_, result);
                //}

                return result;
            }
        };

    public:
        template<typename InputValues>
        [[nodiscard]] consteval static auto make() {
            constexpr auto candidate =
                impl<InputValues>();

            constexpr bool candidate_is_valid = [&](){
               for (auto const entry : InputValues::entries) {
                   if (candidate[entry.key_] != entry.value_) {
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
}
