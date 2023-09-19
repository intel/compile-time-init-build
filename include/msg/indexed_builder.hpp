#pragma once

#include <lookup/entry.hpp>
#include <lookup/input.hpp>
#include <lookup/lookup.hpp>
#include <msg/field_matchers.hpp>
#include <msg/indexed_handler.hpp>
#include <msg/match.hpp>

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
template <typename T> using get_field_type = typename T::field_type;

namespace detail {
template <typename BuilderValue>
CONSTEVAL auto for_each_callback(auto fn) -> void {
    constexpr auto t = BuilderValue::value.callbacks;
    [&]<std::size_t... Is>(std::index_sequence<Is...>) -> void {
        (fn(t[stdx::index<Is>], Is), ...);
    }(std::make_index_sequence<t.size()>{});
}

CONSTEVAL auto with_field_index(auto t) {
    return stdx::transform<get_field_type>([](auto x, auto) { return x; }, t,
                                           t);
}
} // namespace detail

template <typename FieldType, std::size_t EntryCapacity,
          std::size_t CallbackCapacity>
struct temp_index {
    using field_type = FieldType;

    using value_t = stdx::bitset<CallbackCapacity, std::uint32_t>;
    stdx::cx_map<uint32_t, value_t, EntryCapacity> entries{};
    value_t default_value{};

    constexpr auto operator[](auto key) -> auto & {
        if (!entries.contains(key)) {
            entries.put(key, value_t{});
        }
        return entries.get(key);
    }
};

// TODO: needs index configuration
template <typename IndexSpec, typename CallbacksT, typename BaseMsgT,
          typename... ExtraCallbackArgsT>
struct indexed_builder {
    CallbacksT callbacks;

    template <typename... Ts> [[nodiscard]] constexpr auto add(Ts... ts) {
        auto new_callbacks =
            stdx::tuple_cat(callbacks, stdx::make_tuple(ts...));
        using new_callbacks_t = decltype(new_callbacks);
        return indexed_builder<IndexSpec, new_callbacks_t, BaseMsgT,
                               ExtraCallbackArgsT...>{new_callbacks};
    }

    template <typename FieldType, typename T, T... ExpectedValues>
    static CONSTEVAL auto
    get_matchers(in_t<FieldType, T, ExpectedValues...> m) {
        return stdx::make_tuple(m);
    }

    template <typename FieldType, typename T, T ExpectedValue>
    static CONSTEVAL auto
    get_matchers(equal_to_t<FieldType, T, ExpectedValue> m) {
        return stdx::make_tuple(m);
    }

    template <typename... T>
    static CONSTEVAL auto get_matchers(match::all_t<T...> m) {
        return m.matchers;
    }

    using callback_func_t = void (*)(BaseMsgT const &,
                                     ExtraCallbackArgsT... args);

    template <typename BuilderValue, std::size_t I>
    constexpr static auto invoke_callback(BaseMsgT const &msg,
                                          ExtraCallbackArgsT... args) {
        // FIXME: incomplete message callback invocation...
        //        1) bit_cast message argument
        //        2) log message match
        constexpr auto &cb = BuilderValue::value.callbacks[stdx::index<I>];
        if (cb.matcher(msg)) {
            cb.callable(msg, args...);
        }
    }

    template <typename BuilderValue, std::size_t... Is>
    static CONSTEVAL auto create_callback_array(std::index_sequence<Is...>)
        -> std::array<callback_func_t, BuilderValue::value.callbacks.size()> {
        return {invoke_callback<BuilderValue, Is>...};
    }

    template <typename BuilderValue>
    static CONSTEVAL auto create_temp_indices() {
        IndexSpec indices{};
        detail::for_each_callback<BuilderValue>(
            [&](auto callback, auto callback_num) {
                // FIXME: need to convert matcher to product of sums
                auto const matchers =
                    detail::with_field_index(get_matchers(callback.matcher));

                stdx::for_each(
                    [&]<typename T>(T) -> void {
                        using field_type = typename T::field_type;

                        // if this callback specifies a constraint on an indexed
                        // field...
                        if constexpr (stdx::contains_type<decltype(matchers),
                                                          field_type>) {
                            // ...then add that constraint to the index
                            auto const field = get<field_type>(matchers);
                            stdx::for_each(
                                [&](auto field_value) -> void {
                                    get<field_type>(indices)[field_value].set(
                                        callback_num);
                                },
                                field.expected_values);
                        } else {
                            // ...otherwise add this callback to the index's
                            // default value
                            get<field_type>(indices).default_value.set(
                                callback_num);
                        }
                    },
                    indices);
            });
        return indices;
    }

    template <typename I>
    static CONSTEVAL auto get_default_value(auto const &indices) {
        return get<I>(indices).default_value;
    }

    template <typename I, auto E>
    static CONSTEVAL auto get_entry(auto const &indices) {
        return lookup::entry{
            std::next(get<I>(indices).entries.begin(), E)->key,
            std::next(get<I>(indices).entries.begin(), E)->value |
                get<I>(indices).default_value};
    }

    template <typename BuilderValue, typename I, auto... Es>
    static CONSTEVAL auto make_input() {
        struct {
            CONSTEVAL auto operator()() const noexcept {
                constexpr IndexSpec indices =
                    create_temp_indices<BuilderValue>();
                using key_type =
                    typename decltype(get<I>(indices).entries)::key_type;
                using value_type = decltype(get<I>(indices).default_value);
                using entry_t = lookup::entry<key_type, value_type>;
                return lookup::input{get_default_value<I>(indices),
                                     std::array<entry_t, sizeof...(Es)>{
                                         get_entry<I, Es>(indices)...}};
            }
            using cx_value_t [[maybe_unused]] = void;
        } val;
        return val;
    }

    template <typename BuilderValue> static CONSTEVAL auto build() {
        // build callback array
        constexpr auto num_callbacks = BuilderValue::value.callbacks.size();

        constexpr std::array<callback_func_t, num_callbacks> callback_array =
            create_callback_array<BuilderValue>(
                std::make_index_sequence<num_callbacks>{});

        constexpr auto make_index_lookup =
            []<typename I, std::size_t... Es>(std::index_sequence<Es...>) {
                return lookup::make(make_input<BuilderValue, I, Es...>());
            };

        constexpr IndexSpec temp_indices = create_temp_indices<BuilderValue>();
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

        return indexed_handler{
            callback_args_t<BaseMsgT, ExtraCallbackArgsT...>{}, baked_indices,
            callback_array};
    }
};

} // namespace msg
