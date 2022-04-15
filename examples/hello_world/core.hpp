#ifndef CIB_SAY_MESSAGE_HPP
#define CIB_SAY_MESSAGE_HPP


#include <cib/cib.hpp>


struct say_message : public cib::callback_meta<>{};

struct core {
    constexpr static auto config =
        cib::config(cib::exports<say_message>);
};


#endif //CIB_SAY_MESSAGE_HPP
