#include <cib/builder_meta.hpp>
#include <cib/built.hpp>
#include <cib/callback.hpp>
#include <cib/config.hpp>
#include <cib/detail/components.hpp>
#include <cib/detail/config_details.hpp>
#include <cib/detail/config_item.hpp>
#include <cib/detail/constexpr_conditional.hpp>
#include <cib/detail/exports.hpp>
#include <cib/detail/extend.hpp>
#include <cib/detail/nexus_details.hpp>
#include <cib/detail/runtime_conditional.hpp>
#include <cib/func_decl.hpp>
#include <cib/nexus.hpp>

#if __STDC_HOSTED__ == 0
extern "C" auto main() -> int;
#endif

auto main() -> int { return 0; }
