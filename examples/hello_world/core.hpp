#pragma once

#include <cib/cib.hpp>

struct say_message : public callback::service<> {};

struct core {
    constexpr static auto config = cib::config(cib::exports<say_message>);
};
