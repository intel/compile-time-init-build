#include <nexus/callback.hpp>
#include <nexus/service.hpp>

// EXPECT: service has more than one callback requiring a move-only arg

namespace {
struct move_only {
    constexpr move_only() = default;
    constexpr move_only(move_only &&) = default;
    constexpr auto operator=(move_only &&) noexcept -> move_only & = default;
};

template <typename BuilderValue, typename T = void>
constexpr static auto build() {
    return BuilderValue::value.template build<BuilderValue, T>();
}

struct CallbackWithRValueRefArg {
    using service = callback::service<move_only>;

    constexpr static auto value = []() {
        auto const builder = cib::builder_t<service>{};
        return builder.add([](move_only) {}).add([](move_only) {});
    }();
};
} // namespace

auto main() -> int {
    constexpr auto built_callback = build<CallbackWithRValueRefArg>();
    built_callback(move_only{});
}
