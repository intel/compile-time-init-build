#pragma once

#include <seq/build_status.hpp>
#include <seq/step.hpp>

namespace seq {
enum class direction { FORWARD = 0, BACKWARD = 1 };

template <int NumSteps = 0> struct impl {
    func_ptr _forward_steps[NumSteps];
    func_ptr _backward_steps[NumSteps];
    int next_step{};

    status prev_status{};
    direction prev_direction{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    constexpr explicit impl(func_ptr const *forward_steps = nullptr,
                            func_ptr const *backward_steps = nullptr) {
        for (auto i = 0; i < NumSteps; i++) {
            _forward_steps[i] = forward_steps[i];
            _backward_steps[i] = backward_steps[i];
        }

        prev_status = status::DONE;
        prev_direction = direction::BACKWARD;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    constexpr impl(step_base const *steps, build_status) {
        for (auto i = 0; i < NumSteps; i++) {
            _forward_steps[i] = steps[i]._forward_ptr;
            _backward_steps[i] = steps[i]._backward_ptr;
        }

        prev_status = status::DONE;
        prev_direction = direction::BACKWARD;
    }

  private:
    constexpr auto step_forward() -> status {
        if (_forward_steps[next_step]() == status::NOT_DONE) {
            return status::NOT_DONE;
        }
        next_step += 1;
        return status::DONE;
    }

    constexpr auto step_backward() -> status {
        if (_backward_steps[next_step - 1]() == status::NOT_DONE) {
            return status::NOT_DONE;
        }
        next_step -= 1;
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
    [[nodiscard]] static constexpr auto opposite() -> direction {
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
} // namespace seq
