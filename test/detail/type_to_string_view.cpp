#include <cib/cib.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string_view>


struct ABCDEFG;

static_assert(cib::detail::name<ABCDEFG>() == "ABCDEFG");
