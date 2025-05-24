#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: -Wdangling
namespace {
using namespace msg;

using f1 = field<"f1", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using f2 = field<"f2", std::uint32_t>::located<at{0_dw, 7_msb, 0_lsb}>;

using msg_defn = message<"test_msg", f1, f2>;
} // namespace

auto main() -> int {
#if defined(__clang__)
    [[maybe_unused]] auto v = msg::owning<msg_defn>{}.as_const_view();
#else
    STATIC_REQUIRE(false, "-Wdangling");
#endif
}
