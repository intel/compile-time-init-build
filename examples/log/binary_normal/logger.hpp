// include the binary logger
#include <log/catalog/encoder.hpp>

#include <stdx/span.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace custom {
// Provide a destination: a structure with a call operator that takes a
// stdx::span.
struct log_destination {
    // The call operator can be a function template, in which case we will get
    // an instantiation for each span size (roughly corresponding to number of
    // runtime log arguments).
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> packet) const {
        // write the binary log packet somewhere...
        std::cout << "Got a binary log packet\n";
    }

    // Or the call operator can take a dynamic-sized span
    // auto operator()(stdx::span<std::uint32_t const> packet) const {
    // }
};
} // namespace custom

// specialize the logging config variable template to use the binary logger with
// a destination
// remember: each translation unit that logs must see this same specialization!
template <>
inline auto logging::config<> =
    logging::binary::config{custom::log_destination{}};
