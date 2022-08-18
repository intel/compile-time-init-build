#pragma once

#include <container/Vector.hpp>

#include <type_traits>

namespace flow {
    class milestone_base;
}

namespace flow::detail {
    using FlowComboVector = Vector<milestone_base const *, 8>;

    template<typename T>
    [[nodiscard]] constexpr auto convert_milestone_to_ptr(
        T const & value
    ) {
        if constexpr (std::is_base_of_v<milestone_base, T>) {
            return &value;
        } else {
            return value;
        }
    }

    template<typename NameType>
    void log_flow_milestone() {
        TRACE("flow.milestone({})", NameType{});
    }
}





