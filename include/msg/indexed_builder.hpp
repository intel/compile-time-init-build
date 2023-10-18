#pragma once

#include <log/log.hpp>
#include <msg/detail/indexed_builder_common.hpp>
#include <msg/indexed_handler.hpp>

#include <stdx/bitset.hpp>
#include <stdx/compiler.hpp>
#include <stdx/cx_map.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace msg {
// TODO: needs index configuration
template <typename IndexSpec, typename CallbacksT, typename BaseMsgT,
          typename... ExtraCallbackArgsT>
struct indexed_builder
    : indexed_builder_base<indexed_builder, IndexSpec, CallbacksT, BaseMsgT,
                           ExtraCallbackArgsT...> {
    using base_t = indexed_builder_base<indexed_builder, IndexSpec, CallbacksT,
                                        BaseMsgT, ExtraCallbackArgsT...>;

    template <typename I, auto E>
    static CONSTEVAL auto get_entry(auto const &indices) {
        return lookup::entry{
            std::next(get<I>(indices).entries.begin(), E)->key,
            std::next(get<I>(indices).entries.begin(), E)->value};
    }

    template <typename BuilderValue, typename I, auto... Es>
    static CONSTEVAL auto make_input() {
        struct {
            CONSTEVAL auto operator()() const noexcept {
                constexpr IndexSpec indices =
                    base_t::template create_temp_indices<BuilderValue>();
                using key_type =
                    typename decltype(get<I>(indices).entries)::key_type;
                using value_type = decltype(get<I>(indices).default_value);
                using entry_t = lookup::entry<key_type, value_type>;
                return lookup::input{get<I>(indices).default_value,
                                     std::array<entry_t, sizeof...(Es)>{
                                         get_entry<I, Es>(indices)...}};
            }
            using cx_value_t [[maybe_unused]] = void;
        } val;
        return val;
    }
    template <typename BuilderValue> static CONSTEVAL auto build() {
        constexpr auto make_index_lookup =
            []<typename I, std::size_t... Es>(std::index_sequence<Es...>) {
                return lookup::make(make_input<BuilderValue, I, Es...>());
            };

        constexpr IndexSpec temp_indices =
            base_t::template create_temp_indices<BuilderValue>();
        auto const entry_index_seq = [&]<typename I>() {
            return std::make_index_sequence<
                get<I>(temp_indices).entries.size()>{};
        };

        constexpr auto baked_indices =
            temp_indices.apply([&]<typename... I>(I...) {
                return indices{
                    index{typename I::field_type{},
                          make_index_lookup.template operator()<I>(
                              entry_index_seq.template operator()<I>())}...};
            });

        constexpr auto num_callbacks = BuilderValue::value.callbacks.size();
        constexpr std::array<typename base_t::callback_func_t, num_callbacks>
            callback_array =
                base_t::template create_callback_array<BuilderValue>(
                    std::make_index_sequence<num_callbacks>{});

        return indexed_handler{
            callback_args_t<BaseMsgT, ExtraCallbackArgsT...>{}, baked_indices,
            callback_array};
    }
};

} // namespace msg
