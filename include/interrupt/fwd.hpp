#include <flow/flow.hpp>
#include <boost/hana.hpp>

#ifndef CIB_INTERRUPT_FWD_HPP
#define CIB_INTERRUPT_FWD_HPP

namespace interrupt {
    namespace hana = boost::hana;
    using namespace hana::literals;

    using FunctionPtr = std::add_pointer<void()>::type;

    template<typename Name = void>
    using irq_flow = flow::builder<Name, 16, 8>;
}

#endif //CIB_INTERRUPT_FWD_HPP
