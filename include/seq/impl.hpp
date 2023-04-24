#pragma once

#include <seq/build_status.hpp>
#include <seq/step.hpp>

#include <array>
#include <cstddef>

namespace seq {
enum class direction { FORWARD = 0, BACKWARD = 1 };

template <std::size_t NumSteps> struct impl {
    std::array<func_ptr, NumSteps> _forward_steps{};
    std::array<func_ptr, NumSteps> _backward_steps{};
    std::size_t next_step{};

    status prev_status{status::DONE};
    direction prev_direction{direction::BACKWARD};

    constexpr explicit impl(func_ptr const *forward_steps = nullptr,
                            func_ptr const *backward_steps = nullptr) {
        for (auto i = std::size_t{}; i < NumSteps; i++) {
            _forward_steps[i] = forward_steps[i];
            _backward_steps[i] = backward_steps[i];
        }
    }

    constexpr impl(step_base const *steps, build_status) {
        for (auto i = std::size_t{}; i < NumSteps; i++) {
            _forward_steps[i] = steps[i]._forward_ptr;
            _backward_steps[i] = steps[i]._backward_ptr;
        }
    }

  private:
    constexpr auto step_forward() -> status {
        if (_forward_steps[next_step]() == status::NOT_DONE) {
            return status::NOT_DONE;
        }
        ++next_step;
        return status::DONE;
    }

    constexpr auto step_backward() -> status {
        if (_backward_steps[next_step - 1]() == status::NOT_DONE) {
            return status::NOT_DONE;
        }
        --next_step;
        return status::DONE;
    }

    template <direction dir> constexpr auto step() -> status {
        if constexpr (dir == direction::FORWARD) {
            return step_forward();
        } else {
            return step_backward();
        }
    }

    template <direction dir>
    [[nodiscard]] constexpr auto can_continue() const -> bool {
        if constexpr (dir == direction::FORWARD) {
            return next_step < NumSteps;
        } else {
            return next_step > 0;
        }
    }

    template <direction dir>
    [[nodiscard]] constexpr static auto opposite() -> direction {
        if constexpr (dir == direction::FORWARD) {
            return direction::BACKWARD;
        } else {
            return direction::FORWARD;
        }
    }

    template <direction dir> constexpr auto go() -> status {
        constexpr direction opposite_dir = opposite<dir>();

        // check if previous direction has finished or not
        if (prev_direction == opposite_dir && prev_status == status::NOT_DONE &&
            step<opposite_dir>() == status::NOT_DONE) {
            return status::NOT_DONE;
        }

        prev_direction = dir;

        // proceed in the requested direction
        while (can_continue<dir>()) {
            if (step<dir>() == status::NOT_DONE) {
                prev_status = status::NOT_DONE;
                return status::NOT_DONE;
            }
        }

        prev_status = status::DONE;
        return status::DONE;
    }

  public:
    constexpr auto forward() -> status { return go<direction::FORWARD>(); }
    constexpr auto backward() -> status { return go<direction::BACKWARD>(); }
};

template <> struct impl<0u> {
    constexpr impl() = default;
    constexpr impl(step_base const *, build_status) noexcept {}

    constexpr static auto forward() -> status { return status::DONE; }
    constexpr static auto backward() -> status { return status::DONE; }
};

} // namespace seq
