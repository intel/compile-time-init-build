#pragma once

#include <lookup/entry.hpp>
#include <lookup/input.hpp>
#include <lookup/lookup.hpp>
#include <match/and.hpp>
#include <match/concepts.hpp>
#include <match/ops.hpp>
#include <match/sum_of_products.hpp>
#include <msg/callback.hpp>
#include <msg/detail/separate_sum_terms.hpp>
#include <msg/field_matchers.hpp>
#include <sc/string_constant.hpp>

#include <stdx/bitset.hpp>
#include <stdx/compiler.hpp>
#include <stdx/concepts.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/cx_map.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

namespace msg {
struct null_matcher_validator {
    template <match::matcher M>
    CONSTEVAL static auto validate() noexcept -> bool {
        return true;
    }
};

struct never_matcher_validator {
    template <match::matcher M>
    CONSTEVAL static auto validate() noexcept -> bool {
        return not std::is_same_v<M, match::never_t>;
    }
};

template <typename...> inline auto matcher_validator = null_matcher_validator{};

template <match::matcher M, typename... DummyArgs>
CONSTEVAL auto validate_matcher() -> bool {
    return matcher_validator<DummyArgs...>.template validate<M>();
}

template <typename... Fields>
constexpr auto remove_match_terms = []<typename C>(C &&c) {
    using callback_t = std::remove_cvref_t<C>;
    match::matcher auto new_matcher = remove_terms(
        std::forward<C>(c).matcher, std::type_identity<Fields>{}...);
    return detail::callback<callback_t::name, typename callback_t::msg_t,
                            decltype(new_matcher),
                            typename callback_t::callable_t>{
        std::move(new_matcher), std::forward<C>(c).callable};
};

template <typename FieldType, std::size_t EntryCapacity,
          std::size_t CallbackCapacity>
struct temp_index {
    using field_type = FieldType;
    using key_type = typename field_type::value_type;

    using value_t = stdx::bitset<CallbackCapacity, std::uint32_t>;
    stdx::cx_map<key_type, value_t, EntryCapacity> entries{};
    value_t default_value{};
    value_t negative_value{};

    constexpr auto add_positive(key_type key, std::size_t idx) -> void {
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

    constexpr auto add_negative(key_type key, std::size_t idx) -> void {
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
        auto new_callbacks =
            stdx::tuple_cat(callbacks, separate_sum_terms(ts)...);
        using new_callbacks_t = decltype(new_callbacks);
        return ParentT<IndexSpec, new_callbacks_t, BaseMsgT,
                       ExtraCallbackArgsT...>{new_callbacks};
    }

    using callback_func_t = void (*)(BaseMsgT const &,
                                     ExtraCallbackArgsT... args);

    template <typename BuilderValue, std::size_t I>
    constexpr static auto invoke_callback(BaseMsgT const &data,
                                          ExtraCallbackArgsT... args) {
        // FIXME: incomplete message callback invocation...
        //        1) bit_cast message argument
        constexpr auto cb = IndexSpec{}.apply([&]<typename... Indices>(
                                                  Indices...) {
            constexpr auto orig_cb =
                BuilderValue::value.callbacks[stdx::index<I>];
            return remove_match_terms<typename Indices::field_type...>(orig_cb);
        });

        auto const &orig_cb = BuilderValue::value.callbacks[stdx::index<I>];
        using CB = std::remove_cvref_t<decltype(cb)>;
        if constexpr (not validate_matcher<typename CB::matcher_t>()) {
            static_assert(
                stdx::always_false_v<std::remove_cvref_t<decltype(orig_cb)>>,
                "Indexed callback has matcher that is never matched!");
        }

        auto view = typename CB::msg_t::view_t{data};

        if (cb.matcher(view)) {
            CIB_INFO(
                "Incoming message matched [{}], because [{}] (collapsed to "
                "[{}]), executing callback",
                stdx::ct_string_to_type<cb.name, sc::string_constant>(),
                orig_cb.matcher.describe(), cb.matcher.describe());
            cb.callable(view, args...);
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
