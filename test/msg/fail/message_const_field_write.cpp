#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: change a field with a required value
namespace {
using namespace msg;

using f = field<"f", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;

using msg_defn = message<"test_msg", f::with_required<1>>;
} // namespace

auto main() -> int { [[maybe_unused]] msg::owning<msg_defn> m{"f"_field = 2}; }
