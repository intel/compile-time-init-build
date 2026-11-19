#pragma once

#include "common.hpp"

#include <interrupt/config.hpp>

#include <groov/groov.hpp>
#include <groov/test.hpp>

#include <stdx/ct_string.hpp>

#include <cstdint>

using EN_33_1 = groov::field<"33_1", std::uint8_t, 1, 1>;
using EN_33_2 = groov::field<"33_2", std::uint8_t, 2, 2>;

using ST_33_1 = groov::field<"33_1", std::uint8_t, 1, 1>;
using ST_33_2 = groov::field<"33_2", std::uint8_t, 2, 2>;

using R_ENABLE = groov::reg<"enable", std::uint32_t, 1234, groov::w::replace,
                            EN_33_1, EN_33_2>;
using R_STATUS = groov::reg<"status", std::uint32_t, 5678, groov::w::replace,
                            ST_33_1, ST_33_2>;

using G = groov::group<"test", groov::test::bus<"test">, R_ENABLE, R_STATUS>;

struct flow_33_1 : std::true_type {};
struct flow_33_2 : std::true_type {};

using config = interrupt::root<interrupt::shared_irq<
    "shared_33", 33_irq, 34, interrupt::no_field_t, interrupt::policies<>,
    interrupt::sub_irq<"sub_33_1", en_field_t<"33_1">, st_field_t<"33_1">,
                       interrupt::policies<>, flow_33_1>,
    interrupt::sub_irq<"sub_33_2", en_field_t<"33_2">, st_field_t<"33_2">,
                       interrupt::policies<>, flow_33_2>>>;

struct only_flow_33_1_policy_t
    : interrupt::dynamic_init::all_resources_policy,
      interrupt::dynamic_init::flows_policy<flow_33_1>,
      interrupt::dynamic_init::all_irqs_policy {
    constexpr static auto dynamic_enable_top_level = true;
};
