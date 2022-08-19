#include <cib/cib.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string_view>


struct ABCDEFG;

TEST_CASE("ensure types are converted to full string", "[type_to_string_view]") {
    //using std::string_view_literals::sv;
    REQUIRE(cib::detail::name<ABCDEFG>() == "ABCDEFG");
}
