#include <msg/callback.hpp>
#include <msg/detail/indexed_builder_common.hpp>
#include <msg/detail/indexed_handler_common.hpp>
#include <msg/detail/separate_sum_terms.hpp>
#include <msg/field.hpp>
#include <msg/field_matchers.hpp>
#include <msg/handler.hpp>
#include <msg/handler_builder.hpp>
#include <msg/handler_interface.hpp>
#include <msg/indexed_builder.hpp>
#include <msg/indexed_handler.hpp>
#include <msg/indexed_service.hpp>
#include <msg/message.hpp>
#include <msg/send.hpp>
#include <msg/service.hpp>

#if __STDC_HOSTED__ == 0
extern "C" auto main() -> int;
#endif

#ifdef SIMULATE_FREESTANDING
namespace {
struct conc_policy {
    template <typename = void, std::invocable F, std::predicate... Pred>
        requires(sizeof...(Pred) < 2)
    static inline auto call_in_critical_section(F &&f, auto &&...pred)
        -> decltype(auto) {
        while (true) {
            if ((... and pred())) {
                return std::forward<F>(f)();
            }
        }
    }
};
} // namespace

template <> inline auto conc::injected_policy<> = conc_policy{};
#endif

auto main() -> int { return 0; }
