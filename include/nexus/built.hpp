#pragma once

#include <nexus/builder_meta.hpp>

namespace cib {
template <builder_meta ServiceMeta>
constinit auto service = ServiceMeta::uninitialized();
} // namespace cib
