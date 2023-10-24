#pragma once

#include <lookup/entry.hpp>
#include <lookup/input.hpp>
#include <lookup/lookup.hpp>
#include <match/ops.hpp>
#include <msg/field_matchers.hpp>

#include <stdx/bitset.hpp>
#include <stdx/compiler.hpp>
#include <stdx/cx_map.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>

namespace msg {
template <typename FieldType, std::size_t EntryCapacity,
          std::size_t CallbackCapacity>
struct temp_index {
    using field_type = FieldType;

    using value_t = stdx::bitset<CallbackCapacity, std::uint32_t>;
    stdx::cx_map<uint32_t, value_t, EntryCapacity> entries{};
    value_t default_value{};
    value_t negative_value{};

    constexpr auto add_positive(auto key, std::size_t idx) -> void {
        // add this index into the map: simple
        if (not entries.contains(key)) {
            entries.put(key, value_t{});
        }
        entries.get(key).set(idx);
    }

    constexpr auto collect_defaults(std::size_t max) -> void {
        // each index not represented under any key goes into the defaults
        for (auto idx = std::size_t{}; idx < max; ++idx) {
            if (std::none_of(std::cbegin(entries), std::cend(entries),
                             [&](auto const &entry) -> bool {
                                 return entry.value[idx];
                             })) {
                default_value.set(idx);
            }
        }
    }

    constexpr auto add_negative(auto key, std::size_t idx) -> void {
        // the defaults (without this index) go in the entry for this key
        if (not entries.contains(key)) {
            entries.put(key, value_t{});
        }
        auto &entry = entries.get(key);
        entry |= default_value;
        entry.reset(idx);
        // this index goes under all the other keys
        for (auto &[k, v] : entries) {
            if (k != key) {
                v.set(idx);
            }
        }
        // and we'll accumulate the negatives
        negative_value.set(idx);
    }

    constexpr auto propagate_positive_defaults() -> void {
        // the "positive defaults" are the defaults without the negatives
        auto const def = default_value & ~negative_value;
        // and they get propagated to each entry
        for (auto &[k, v] : entries) {
            v |= def;
        }
    }
};

template <typename T> using get_field_type = typename T::field_type;

template <typename... Fields>
using index_spec = decltype(stdx::make_indexed_tuple<get_field_type>(
    temp_index<Fields, 256, 32>{}...));

template <template <typename, typename, typename, typename...> typename ParentT,
          typename IndexSpec, typename CallbacksT, typename BaseMsgT,
          typename... ExtraCallbackArgsT>
struct indexed_builder_base {
    CallbacksT callbacks;

    template <typename... Ts> [[nodiscard]] constexpr auto add(Ts... ts) {
        [[maybe_unused]] auto const msg_matcher =
            typename BaseMsgT::matcher_t{};
        auto new_callbacks =
            stdx::tuple_cat(callbacks, separate_sum_terms(ts, msg_matcher)...);
        using new_callbacks_t = decltype(new_callbacks);
        return ParentT<IndexSpec, new_callbacks_t, BaseMsgT,
                       ExtraCallbackArgsT...>{new_callbacks};
    }

    using callback_func_t = void (*)(BaseMsgT const &,
                                     ExtraCallbackArgsT... args);

    template <typename BuilderValue, std::size_t I>
    constexpr static auto invoke_callback(BaseMsgT const &msg,
                                          ExtraCallbackArgsT... args) {
        // FIXME: incomplete message callback invocation...
        //        1) bit_cast message argument
        constexpr auto &orig_cb = BuilderValue::value.callbacks[stdx::index<I>];
        constexpr auto cb = IndexSpec{}.apply([&]<typename... Indices>(
                                                  Indices...) {
            return remove_match_terms<typename Indices::field_type...>(orig_cb);
        });
        if (cb.matcher(msg)) {
            CIB_INFO(
                "Incoming message matched [{}], because [{}] (collapsed to "
                "[{}]), executing callback",
                cb.name, orig_cb.matcher.describe(), cb.matcher.describe());
            cb.callable(msg, args...);
        }
    }

    template <typename BuilderValue, std::size_t... Is>
    static CONSTEVAL auto create_callback_array(std::index_sequence<Is...>)
        -> std::array<callback_func_t, BuilderValue::value.callbacks.size()> {
        return {invoke_callback<BuilderValue, Is>...};
    }

    static CONSTEVAL auto walk_matcher(auto const &tag, auto const &callbacks,
                                       auto const &f) {
        auto idx = std::size_t{};
        stdx::for_each([&](auto callback) { tag(callback.matcher, f, idx++); },
                       callbacks);
    }

    template <typename BuilderValue>
    static CONSTEVAL auto create_temp_indices() {
        IndexSpec indices{};
        walk_matcher(index_terms, BuilderValue::value.callbacks,
                     [&]<typename Field>(std::size_t idx, auto expected_value) {
                         if constexpr (stdx::contains_type<IndexSpec, Field>) {
                             get<Field>(indices).add_positive(expected_value,
                                                              idx);
                         }
                     });
        [[maybe_unused]] constexpr auto count =
            BuilderValue::value.callbacks.size();
        stdx::for_each([](auto &index) { index.collect_defaults(count); },
                       indices);
        walk_matcher(index_not_terms, BuilderValue::value.callbacks,
                     [&]<typename Field>(std::size_t idx, auto expected_value) {
                         if constexpr (stdx::contains_type<IndexSpec, Field>) {
                             get<Field>(indices).add_negative(expected_value,
                                                              idx);
                         }
                     });
        stdx::for_each([](auto &index) { index.propagate_positive_defaults(); },
                       indices);
        return indices;
    }
};
} // namespace msg
