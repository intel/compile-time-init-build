// include the binary logger
#include <log_binary/catalog/encoder.hpp>

#include <stdx/span.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace custom {
// Provide a destination: a structure with a call operator that takes a
// stdx::span.
struct log_destination {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> packet) const {
        using namespace msg;
        const_view<logging::mipi::defn::short32_msg_t> m{packet};
        std::cout << "Got a binary log packet, string ID: "
                  << m.get("payload"_field) << '\n';
    }
};
} // namespace custom

// specialize the logging config variable template to use the binary logger with
// a destination
// remember: each translation unit that logs must see this same specialization!
template <>
inline auto logging::config<> =
    logging::binary::config{custom::log_destination{}};
