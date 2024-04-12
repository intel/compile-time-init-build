#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: no matching
namespace {
using namespace msg;

using test_field1 =
    field<"f1", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using test_field2 = field<"f2", std::uint32_t>::located<at{1_dw, 7_msb, 0_lsb}>;

using msg_defn = message<"test_msg", test_field1, test_field2>;

using larger_span_t = stdx::span<std::uint32_t const, 4>;
using larger_view_t = msg_defn::view_t<larger_span_t>;
} // namespace

auto main() -> int {
    owning<msg_defn> m{};
    const_view<msg_defn> v{m};
    larger_view_t lv{v};
}
