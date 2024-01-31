#include <msg/callback.hpp>
#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: Named field not in message
namespace {
using namespace msg;

using test_id_field =
    field<"id", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;

using msg_defn = message<"test_msg", test_id_field>;

constexpr auto test_callback = msg::callback<"test_callback", msg_defn>(
    "none"_field == constant<0x80>, [](auto) {});

int main() {}
