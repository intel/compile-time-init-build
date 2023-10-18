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

    constexpr auto collect_defaults(std::size_t max) {
        for (auto i = std::size_t{}; i < max; ++i) {
            if (std::none_of(
                    std::cbegin(entries), std::cend(entries),
                    [&](auto const &entry) { return entry.value[i]; })) {
                default_value.set(i);
            }
        }
    }
};

template <template <typename, typename, typename, typename...> typename ParentT,
          typename IndexSpec, typename CallbacksT, typename BaseMsgT,
          typename... ExtraCallbackArgsT>
struct indexed_builder_base {
    CallbacksT callbacks;

    template <typename... Ts> [[nodiscard]] constexpr auto add(Ts... ts) {
        auto new_callbacks =
            stdx::tuple_cat(callbacks, stdx::make_tuple(ts...));
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
            CIB_INFO("Incoming message matched [{}], because [{}], executing "
                     "callback",
                     cb.name, orig_cb.matcher.describe());
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
        std::size_t count{};
        stdx::for_each(
            [&](auto callback) {
                build_index(
                    callback.matcher,
                    [&]<typename Field, typename V>(std::size_t idx,
                                                    V expected_value) {
                        if constexpr (stdx::contains_type<IndexSpec, Field>) {
                            stdx::get<Field>(indices)[expected_value].set(idx);
                        }
                    },
                    count);
            },
            BuilderValue::value.callbacks);
        stdx::for_each(
            [n = ++count](auto &index) { index.collect_defaults(n); }, indices);
        return indices;
    }
};

} // namespace msg
