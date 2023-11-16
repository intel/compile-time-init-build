#pragma once

#include <log/log.hpp>
#include <seq/step.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/cx_vector.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <span>

namespace seq {
enum struct direction { FORWARD = 0, BACKWARD = 1 };

template <stdx::ct_string, std::size_t NumSteps> struct impl {
    stdx::cx_vector<func_ptr, NumSteps> _forward_steps{};
    stdx::cx_vector<func_ptr, NumSteps> _backward_steps{};
    std::size_t next_step{};

    status prev_status{status::DONE};
    direction prev_direction{direction::BACKWARD};

    using node_t = rt_step;

    constexpr explicit(true) impl(std::span<node_t const> steps) {
        CIB_ASSERT(NumSteps >= std::size(steps));
        for (auto const &step : steps) {
            _forward_steps.push_back(step.forward_ptr);
            _backward_steps.push_back(step.backward_ptr);
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
} // namespace seq
