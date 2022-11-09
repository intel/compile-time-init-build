#ifndef COMPILE_TIME_INIT_BUILD_BUILDER_META_HPP
#define COMPILE_TIME_INIT_BUILD_BUILDER_META_HPP

namespace cib {
/**
 * Describe a builder to cib.
 *
 * @tparam BuilderType
 *      The initial builder type cib should use when creating a builder.
 *      This is only the initial type, the BuilderType::add(...) function
 *      may return a different type and cib will track that correctly.
 *
 * @tparam InterfaceType
 *      The type-erased interface services built with this builder
 *      will implement. For example, cib::callback allows many other
 *      callables to get executed when its service gets invoked. The
 *      type-erased interface for cib::callback is a function pointer.
 *
 * @see cib::built
 *
 * @example cib::callback_meta
 */
template <typename BuilderType, typename InterfaceType> struct builder_meta {
    auto builder() -> BuilderType;
    auto interface() -> InterfaceType;
};
} // namespace cib

#endif // COMPILE_TIME_INIT_BUILD_BUILDER_META_HPP
