#pragma once

#include <cib/builder_meta.hpp>

namespace cib {
/**
 * Pointer to a built service implementation.
 *
 * @tparam ServiceMeta
 *      Tag name of the service.
 *
 * @see cib::builder_meta
 */
template <typename ServiceMeta> interface_t<ServiceMeta> service;
} // namespace cib
