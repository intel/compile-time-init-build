#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: consider using equivalent
namespace {
using namespace msg;

using test_field1 =
    field<"f1", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using test_field2 = field<"f2", std::uint32_t>::located<at{1_dw, 7_msb, 0_lsb}>;

using msg_defn = message<"test_msg", test_field1, test_field2>;
} // namespace

auto main() -> int {
    owning<msg_defn> m{};
    auto mv1 = m.as_mutable_view();
    auto mv2 = m.as_mutable_view();
    [[maybe_unused]] auto b = mv1 == mv2;
}
