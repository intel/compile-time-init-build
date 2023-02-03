#include <cib/cib.hpp>
#include <msg/service.hpp>

#include <catch2/catch_test_macros.hpp>

namespace msg {

using test_id_field =
    field<decltype("test_id_field"_sc), 0, 31, 24, std::uint32_t>;
using test_field_1 =
    field<decltype("test_field_1"_sc), 0, 15, 0, std::uint32_t>;
using test_field_2 =
    field<decltype("test_field_2"_sc), 1, 23, 16, std::uint32_t>;
using test_field_3 =
    field<decltype("test_field_3"_sc), 1, 15, 0, std::uint32_t>;

using test_msg_t = message_base<decltype("test_msg"_sc), 4, 2,
                                test_id_field::WithRequired<0x80>, test_field_1,
                                test_field_2, test_field_3>;

struct test_service : ::msg::service<test_msg_t> {};

static inline bool callback_success;

static constexpr auto test_callback = msg::callback<test_msg_t>(
    "TestCallback"_sc, match::always<true>,
    [](test_msg_t const &) { callback_success = true; });

struct test_project {
    constexpr static auto config = cib::config(
        cib::exports<test_service>, cib::extend<test_service>(test_callback));
};

TEST_CASE("build handler", "[handler_builder]") {
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();

    callback_success = false;

    cib::service<test_service>->handle(test_msg_t{test_id_field{0x80}});

    REQUIRE(callback_success);
}

} // namespace msg
