#pragma once

#include <cib/cib.hpp>

struct say_message : public cib::callback_meta<> {};

struct core {
    constexpr static auto config = cib::config(cib::exports<say_message>);
};
