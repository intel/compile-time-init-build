#ifndef SIMULATE_FREESTANDING
#include <log_fmt/logger.hpp>
#endif

#if __STDC_HOSTED__ == 0
extern "C" auto main() -> int;
#endif

auto main() -> int { return 0; }
