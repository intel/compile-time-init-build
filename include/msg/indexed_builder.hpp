#pragma once

#include <cib/builder_meta.hpp>
#include <cib/tuple.hpp>
#include <container/constexpr_map.hpp>
#include <container/vector.hpp>
#include <log/log.hpp>
#include <lookup/lookup.hpp>
#include <msg/detail/bitset.hpp>
#include <msg/field_matchers.hpp>
#include <msg/indexed_handler.hpp>
#include <msg/match.hpp>
#include <msg/message.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

namespace msg {
template <typename T> using get_field_type = typename T::field_type;

namespace detail {
template <typename BuilderValue>
consteval auto for_each_callback(auto fn) -> void {
    constexpr auto t = BuilderValue::value.callbacks;
    [&]<std::size_t... i>(std::index_sequence<i...>)->void {
        (fn(t[cib::index<i>], i), ...);
    }
    (std::make_index_sequence<t.size()>{});
}

consteval auto with_field_index(auto t) {
    return cib::transform<get_field_type>([](auto x, auto) { return x; }, t, t);
}
} // namespace detail

template <typename FieldType, std::size_t EntryCapacity,
          std::size_t CallbackCapacity>
struct temp_index {
    using field_type = FieldType;

    cib::constexpr_map<uint32_t, detail::bitset<CallbackCapacity>,
                       EntryCapacity>
        entries{};
    detail::bitset<CallbackCapacity> default_value{};

    constexpr auto &operator[](auto key) {
        if (!entries.contains(key)) {
            entries.put(key, {});
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
        auto new_callbacks = cib::tuple_cat(callbacks, cib::make_tuple(ts...));
        using new_callbacks_t = decltype(new_callbacks);
        return indexed_builder<IndexSpec, new_callbacks_t, BaseMsgT,
                               ExtraCallbackArgsT...>{new_callbacks};
    }

    template <typename FieldType, typename T, T... ExpectedValues>
    static consteval auto
    get_matchers(in_t<FieldType, T, ExpectedValues...> m) {
        return cib::make_tuple(m);
    }

    template <typename FieldType, typename T, T ExpectedValue>
    static consteval auto
    get_matchers(equal_to_t<FieldType, T, ExpectedValue> m) {
        return cib::make_tuple(m);
    }

    template <typename... T>
    static consteval auto get_matchers(match::all_t<T...> m) {
        return m.matchers;
    }

    using callback_func_t = void (*)(BaseMsgT const &,
                                     ExtraCallbackArgsT... args);

    template <typename BuilderValue, std::size_t... i>
    static consteval auto create_callback_array(std::index_sequence<i...>)
        -> std::array<callback_func_t, BuilderValue::value.callbacks.size()> {
        return {// FIXME: incomplete message callback invocation...
                //        1) bit_cast message argument
                //        2) log message match
                //        3) unindexed fields checking
                BuilderValue::value.callbacks[cib::index<i>].callable...};
    }

    template <typename BuilderValue>
    static consteval auto create_temp_indices() {
        IndexSpec indices{};
        detail::for_each_callback<BuilderValue>([&](auto callback,
                                                    auto callback_num) {
            // FIXME: need to convert matcher to product of sums
            auto const matchers =
                detail::with_field_index(get_matchers(callback.matcher));

            for_each(
                [&]<typename T>(T) -> void {
                    using field_type = typename T::field_type;

                    // if this callback specifies a constraint on an indexed
                    // field...
                    if constexpr (cib::contains_type<decltype(matchers),
                                                     field_type>) {
                        // ...then add that constraint to the index
                        auto const field = matchers.get(cib::tag<field_type>);
                        for_each(
                            [&](auto field_value) -> void {
                                indices.get(cib::tag<field_type>)[field_value]
                                    .add(callback_num);
                            },
                            field.expected_values);
                    } else {
                        // ...otherwise add this callback to the index's default
                        // value
                        indices.get(cib::tag<field_type>)
                            .default_value.add(callback_num);
                    }
                },
                indices);
        });
        return indices;
    }

    template <typename BuilderValue, typename I, auto entry_num>
    static consteval auto make_entry() {
        constexpr IndexSpec temp_indices = create_temp_indices<BuilderValue>();

        return lookup::entry{
            std::next(temp_indices.get(cib::tag<I>).entries.begin(), entry_num)
                ->key,
            std::next(temp_indices.get(cib::tag<I>).entries.begin(), entry_num)
                    ->value |
                temp_indices.get(cib::tag<I>).default_value};
    }

    template <typename BuilderValue> static consteval auto build() {
        // build callback array
        constexpr auto num_callbacks = BuilderValue::value.callbacks.size();

        constexpr std::array<callback_func_t, num_callbacks> callback_array =
            create_callback_array<BuilderValue>(
                std::make_index_sequence<num_callbacks>{});

        // build temporary indices for each field
        constexpr IndexSpec temp_indices = create_temp_indices<BuilderValue>();

        auto const make_index_lookup = [&]<typename I,
                                           std::size_t... entry_num>(
                                           cib::tag_constant<I> *,
                                           std::index_sequence<entry_num...>) {
            return lookup::make<lookup::input<
                uint32_t, decltype(temp_indices.get(cib::tag<I>).default_value),
                temp_indices.get(cib::tag<I>).default_value,
                make_entry<BuilderValue, I, entry_num>()...>>();
        };

        auto const entry_index_seq = [&]<typename I>(cib::tag_constant<I> *) {
            return std::make_index_sequence<
                temp_indices.get(cib::tag<I>).entries.size()>{};
        };

        constexpr auto baked_indices =
            temp_indices.apply([&]<typename... I>(I...) {
                return indices{
                    index{typename I::field_type{},
                          make_index_lookup(cib::tag<I>,
                                            entry_index_seq(cib::tag<I>))}...};
            });

        return indexed_handler{
            callback_args_t<BaseMsgT, ExtraCallbackArgsT...>{}, baked_indices,
            callback_array};
    }
};

} // namespace msg
