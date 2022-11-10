#pragma once

#include <cib/detail/builder_traits.hpp>

namespace cib {
/**
 * Pointer to a built service implementation.
 *
 * @tparam ServiceMeta
 *      Tag name of the service.
 *
 * @see cib::builder_meta
 */
template <typename ServiceMeta> traits::interface_t<ServiceMeta> service;
} // namespace cib
