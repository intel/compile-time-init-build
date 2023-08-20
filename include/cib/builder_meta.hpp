#pragma once

namespace cib {
/**
 * Describe a builder to cib.
 *
 * @tparam Builder
 *      The initial builder type cib should use when creating a builder.
 *      This is only the initial type, the Builder::add(...) function
 *      may return a different type and cib will track that correctly.
 *
 * @tparam Interface
 *      The type-erased interface services built with this builder
 *      will implement. For example, cib::callback allows many other
 *      callables to get executed when its service gets invoked. The
 *      type-erased interface for cib::callback is a function pointer.
 *
 * @see cib::built
 *
 * @example cib::callback_meta
 */
template <typename Builder, typename Interface> struct builder_meta {
    using builder_t = Builder;
    using interface_t = Interface;
};

template <typename BuilderMeta>
using builder_t = typename BuilderMeta::builder_t;

template <typename BuilderMeta>
using interface_t = typename BuilderMeta::interface_t;
} // namespace cib
