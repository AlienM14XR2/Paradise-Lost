/**
 * サーバを介さないテストを行う。
 * 
 * e.g. compile.
 * g++ -O3 -DDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ test.cpp -o ../bin/test
*/
#include <iostream>
#include <cassert>
#include "rest_api_debug.hpp"

int test_debug_and_error() {
    puts("=== test_debug_and_error");
    try {
        // ptr_print_now();
        auto pi = 3.141592;
        ptr_api_debug<const char*,const decltype(pi)&>("pi is ", pi);
        ptr_api_debug<const char*,const decltype(pi)*>("pi addr is ", &pi);
        throw std::runtime_error("It's test error.");
        return EXIT_SUCCESS;
    } catch(std::exception& e) {
        // printf("e type is ... %s\n", typeid(e).name());
        ptr_api_debug<const char*, const std::string&>("e type is ... ", typeid(e).name());
        ptr_api_error<const decltype(e)&>(e);
        return EXIT_FAILURE;
    }
}

int main(void) {
    puts("=== START Test");
    if(0.01) {
        auto ret = 0;
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_debug_and_error());
        assert(ret == 1);
    }
    puts("END   Test === ");    
}