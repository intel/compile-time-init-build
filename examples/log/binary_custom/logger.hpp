// include the binary logger and message machinery
#include <log/catalog/encoder.hpp>
#include <msg/message.hpp>

#include <stdx/span.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace custom {
namespace defn {
using msg::at;
using msg::dword_index_t;
using msg::field;
using msg::message;
using msg::operator""_msb;
using msg::operator""_lsb;

// Define a message type for the custom binary format.
// For simplicity, this message is just the 32-bit string ID.
using id_f =
    field<"id", std::uint32_t>::located<at{dword_index_t{0}, 31_msb, 0_lsb}>;
using id_msg_t = message<"id", id_f>;
} // namespace defn

// Provide a builder: a structure with a build function that takes
// various arguments and returns an (owning) message.
struct builder : logging::mipi::default_builder<> {
    template <auto Level, logging::packable... Ts>
    static auto build(string_id, module_id, logging::mipi::unit_t, Ts...) {
        using namespace msg;
        return owning<defn::id_msg_t>{"id"_field = 42};
    }
};

// Provide a destination: a structure with a call operator that takes a
// stdx::span.
struct log_destination {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> packet) const {
        // write the binary log packet somewhere...
        std::cout << "Got a binary log packet, string ID: " << packet[0]
                  << '\n';
        ;
    }
};
} // namespace custom

// Specialize the logging config variable template to use the binary logger with
// a destination.
// Remember: each translation unit that logs must see this same specialization!
template <>
inline auto logging::config<> =
    logging::binary::config{custom::log_destination{}};
