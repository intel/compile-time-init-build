#pragma once

#include <msg/message.hpp>

namespace logging::mipi {
namespace defn {
using msg::at;
using msg::dword_index_t;
using msg::field;
using msg::message;
using msg::operator""_msb;
using msg::operator""_lsb;

enum struct type : uint8_t { Build = 0, Short32 = 1, Catalog = 3 };
enum struct build_subtype : uint8_t { Compact32 = 0, Compact64 = 1, Long = 2 };
enum struct catalog_subtype : uint8_t { Id32_Pack32 = 1 };

using type_f = field<"type", type>::located<at{dword_index_t{0}, 3_msb, 0_lsb}>;
using opt_len_f =
    field<"opt_len", bool>::located<at{dword_index_t{0}, 9_msb, 9_lsb}>;
using payload_len_f =
    field<"payload_len",
          std::uint16_t>::located<at{dword_index_t{1}, 15_msb, 0_lsb}>;

using build_subtype_f =
    field<"subtype",
          build_subtype>::located<at{dword_index_t{0}, 29_msb, 24_lsb}>;
using compact32_build_id_f = field<"build_id", std::uint32_t>::located<
    at{dword_index_t{0}, 31_msb, 30_lsb}, at{dword_index_t{0}, 23_msb, 4_lsb}>;
using compact64_build_id_f = field<"build_id", std::uint64_t>::located<
    at{dword_index_t{1}, 31_msb, 0_lsb}, at{dword_index_t{0}, 31_msb, 30_lsb},
    at{dword_index_t{0}, 23_msb, 4_lsb}>;

using normal_build_msg_t =
    message<"normal_build", type_f::with_required<type::Build>,
            opt_len_f::with_required<true>,
            build_subtype_f::with_required<build_subtype::Long>, payload_len_f>;
using compact32_build_msg_t =
    message<"compact32_build", type_f::with_required<type::Build>,
            build_subtype_f::with_required<build_subtype::Compact32>,
            compact32_build_id_f>;
using compact64_build_msg_t =
    message<"compact64_build", type_f::with_required<type::Build>,
            build_subtype_f::with_required<build_subtype::Compact64>,
            compact64_build_id_f>;

using short32_payload_f =
    field<"payload",
          std::uint32_t>::located<at{dword_index_t{0}, 31_msb, 4_lsb}>;
using short32_msg_t =
    message<"short32", type_f::with_required<type::Short32>, short32_payload_f>;

using catalog_subtype_f =
    field<"subtype",
          catalog_subtype>::located<at{dword_index_t{0}, 29_msb, 24_lsb}>;
using severity_f = field<"severity", std::uint8_t>::located<at{dword_index_t{0},
                                                               6_msb, 4_lsb}>;
using module_id_f =
    field<"module_id",
          std::uint8_t>::located<at{dword_index_t{0}, 22_msb, 16_lsb}>;

using catalog_msg_t =
    message<"catalog", type_f::with_required<type::Catalog>, severity_f,
            module_id_f,
            catalog_subtype_f::with_required<catalog_subtype::Id32_Pack32>>;
} // namespace defn
} // namespace logging::mipi
