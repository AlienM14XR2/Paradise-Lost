#ifndef _RESTAPIDEBUG_H_
#define _RESTAPIDEBUG_H_

#include <iostream>

template <class M, class D>
void (*ptr_api_debug)(M, D) = [](const auto message, const auto debug) -> void {
    std::cout << "DEBUG: " << message << '\t' << debug << std::endl;
};

template <class Err>
concept ErrReasonable = requires(Err& e) {
    e.what();
};
template <class Err>
requires ErrReasonable<Err>
void (*ptr_api_error)(Err) = [](const auto e) -> void {
    std::cerr << "ERROR: " << e.what() << std::endl;
};


#endif
