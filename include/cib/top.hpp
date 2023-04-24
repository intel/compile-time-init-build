#pragma once

#include <cib/config.hpp>
#include <cib/nexus.hpp>
#include <flow/flow.hpp>

namespace cib {
/**
 * Executed immediately after the C++ runtime is stable. This should be
 * used to initialize essential services like logging and potentially
 * configure the host system the project is running on.
 */
class EarlyRuntimeInit : public flow::service<> {};

/**
 * Executed once after essential services like logging are initialized.
 * This can be used for general component runtime initialization.
 */
class RuntimeInit : public flow::service<> {};

/**
 * Executed after all runtime initialization is completed. This is where
 * the project can start doing its job including enabling interrupts and/or
 * starting threads if applicable and be ready to start accepting and
 * processing external events.
 */
class RuntimeStart : public flow::service<> {};

/**
 * Executed repeated in an infinite loop after initialization and
 * RuntimeStart flows have completed.
 */
class MainLoop : public flow::service<> {};

/**
 * The top object for cib framework. Call 'main' to execute the project.
 */
template <typename ProjectConfig> class top {
  private:
    struct component {
        constexpr static auto config = cib::config(
            cib::exports<EarlyRuntimeInit, RuntimeInit, RuntimeStart, MainLoop>,
            cib::components<ProjectConfig>);
    };

    constexpr static cib::nexus<component> my_nexus{};

  public:
    /**
     * Main entry point to cib top.
     */
    [[noreturn]] inline void main() {
        my_nexus.init();
        flow::run<EarlyRuntimeInit>();

        CIB_INFO("cib::top::init() - RuntimeInit");
        flow::run<RuntimeInit>();

        CIB_INFO("cib::top::init() - RuntimeStart");
        flow::run<RuntimeStart>();

        while (true) {
            flow::run<MainLoop>();
        }
    }

    template <typename ServiceMeta>
    constexpr static auto get_service() -> auto & {
        return my_nexus.template service<ServiceMeta>;
    }
};
} // namespace cib
