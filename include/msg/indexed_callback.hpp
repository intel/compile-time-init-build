#pragma once

namespace msg {

template<typename Name, typename Matcher, typename Callable>
struct indexed_callback_t {
    constexpr static Name name{};

    Matcher matcher;
    Callable callable;

    consteval indexed_callback_t(
        Name,
        Matcher matcher_arg,
        Callable callable_arg
    )
        : matcher{matcher_arg}
        , callable{callable_arg}
    {}
};

}
