#include <seq/step.hpp>
#include <seq/build_status.hpp>

#ifndef CIB_SEQ_IMPL_HPP
#define CIB_SEQ_IMPL_HPP
namespace seq {
    enum class direction {
        FORWARD = 0,
        BACKWARD = 1
    };

    template <int NumSteps = 0>
    struct impl {
        func_ptr _forward_steps[NumSteps];
        func_ptr _backward_steps[NumSteps];
        int next_step = 0;

        status prev_status;
        direction prev_direction;

        constexpr impl(
            func_ptr forward_steps[] = {},
            func_ptr backward_steps[] = {}
        ) {
            for (auto i = 0; i < NumSteps; i++) {
                _forward_steps[i] = forward_steps[i];
            }

            for (auto i = 0; i < NumSteps; i++) {
                _backward_steps[i] = backward_steps[i];
            }

            prev_status = status::DONE;
            prev_direction = direction::BACKWARD;
        }

        constexpr impl(
            step_base steps[],
            build_status built_status
        ) {
            for (auto i = 0; i < NumSteps; i++) {
                _forward_steps[i] = steps[i]._forward_ptr;
            }

            for (auto i = 0; i < NumSteps; i++) {
                _backward_steps[i] = steps[i]._backward_ptr;
            }

            prev_status = status::DONE;
            prev_direction = direction::BACKWARD;
        }

    private:
        constexpr status step_forward() {
            if (_forward_steps[next_step]() == status::NOT_DONE) {
                return status::NOT_DONE;
            } else {
                next_step += 1;
                return status::DONE;
            }
        }

        constexpr status step_backward() {
            if (_backward_steps[next_step - 1]() == status::NOT_DONE) {
                return status::NOT_DONE;
            } else {
                next_step -= 1;
                return status::DONE;
            }
        }

        template<direction dir>
        constexpr status step() {
            if constexpr (dir == direction::FORWARD) {
                return step_forward();
            } else {
                return step_backward();
            }
        }

        template<direction dir>
        constexpr bool can_continue() {
            if constexpr (dir == direction::FORWARD) {
                return next_step < NumSteps;
            } else {
                return next_step > 0;
            }
        }

        template<direction dir>
        static constexpr direction opposite() {
            if constexpr (dir == direction::FORWARD) {
                return direction::BACKWARD;
            } else {
                return direction::FORWARD;
            }
        }

        template<direction dir>
        constexpr status go() {
            constexpr direction opposite_dir = opposite<dir>();

            // check if previous direction has finished or not
            if (
                prev_direction == opposite_dir &&
                prev_status == status::NOT_DONE &&
                step<opposite_dir>() == status::NOT_DONE
            ) {
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
        constexpr status forward() {
            return go<direction::FORWARD>();
        }

        constexpr status backward() {
            return go<direction::BACKWARD>();
        }
    };
}
#endif //CIB_SEQ_IMPL_HPP
