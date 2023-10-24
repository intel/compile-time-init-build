#include <cib/cib.hpp>
#include <match/ops.hpp>
#include <msg/field.hpp>
#include <msg/indexed_callback.hpp>
#include <msg/indexed_service.hpp>
#include <msg/message.hpp>

namespace {
using test_id_field =
    msg::field<decltype("test_id_field"_sc), 0, 31, 24, std::uint32_t>;

using test_msg_t = msg::message_base<decltype("test_msg"_sc), 2, test_id_field>;

constexpr auto test_callback = msg::indexed_callback(
    "test_callback"_sc,
    test_id_field::equal_to<0x80> and test_id_field::equal_to<0x81>,
    [](test_msg_t const &) {});

using index_spec = msg::index_spec<test_id_field>;
struct test_service : msg::indexed_service<index_spec, test_msg_t> {};

struct test_project {
    constexpr static auto config = cib::config(
        cib::exports<test_service>, cib::extend<test_service>(test_callback));
};
} // namespace

template <>
inline auto msg::matcher_validator<> = msg::never_matcher_validator{};

int main() {
    cib::nexus<test_project> test_nexus{};
    test_nexus.init();
}
