#include <msg/field.hpp>
#include <msg/message.hpp>

// EXPECT: Attempted to construct owning message with incompatible storage
namespace {
using namespace msg;

using test_field1 =
    field<"f1", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using test_field2 = field<"f2", std::uint32_t>::located<at{1_dw, 7_msb, 0_lsb}>;

using msg_defn = message<"test_msg", test_field1, test_field2>;

using uint8_storage_t = msg_defn::custom_storage_t<std::array, std::uint8_t>;
using owning_uint8_msg = msg_defn::owner_t<uint8_storage_t>;
} // namespace

auto main() -> int {
    owning_uint8_msg m{};
    auto const viewu8 = m.as_const_view();
    owning<msg_defn> o{viewu8};
}
