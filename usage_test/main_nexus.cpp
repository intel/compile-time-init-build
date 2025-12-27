#include <nexus/callback.hpp>
#include <nexus/config.hpp>
#include <nexus/detail/components.hpp>
#include <nexus/detail/config_details.hpp>
#include <nexus/detail/config_item.hpp>
#include <nexus/detail/constexpr_conditional.hpp>
#include <nexus/detail/exports.hpp>
#include <nexus/detail/extend.hpp>
#include <nexus/detail/nexus_details.hpp>
#include <nexus/detail/runtime_conditional.hpp>
#include <nexus/func_decl.hpp>
#include <nexus/nexus.hpp>
#include <nexus/service.hpp>

#if __STDC_HOSTED__ == 0
extern "C" auto main() -> int;
#endif

auto main() -> int { return 0; }
