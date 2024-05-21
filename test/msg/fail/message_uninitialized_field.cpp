#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: All fields must be initialized or defaulted
namespace {
using namespace msg;

using f = field<"f", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;

using msg_defn = message<"test_msg", f::without_default>;
} // namespace

auto main() -> int { [[maybe_unused]] msg::owning<msg_defn> m{}; }
