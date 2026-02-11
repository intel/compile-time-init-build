#include <interrupt/concepts.hpp>
#include <interrupt/config.hpp>
#include <interrupt/dynamic_controller.hpp>
#include <interrupt/fwd.hpp>
#include <interrupt/impl.hpp>
#include <interrupt/manager.hpp>
#include <interrupt/policies.hpp>

#if __STDC_HOSTED__ == 0
extern "C" auto main() -> int;
#endif

auto main() -> int { return 0; }
