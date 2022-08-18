#pragma once


#include <cib/built.hpp>

#include <utility>


namespace flow {
    namespace detail {
        using FunctionPtr = std::add_pointer<void()>::type;
    }

    /**
     * Run the flow given by 'Tag'.
     *
     * @tparam Tag Type of the flow to be ran. This is the name of the flow::Builder used to declare and build the flow.
     */
    template<typename Tag>
    detail::FunctionPtr & run = cib::service<Tag>;
}