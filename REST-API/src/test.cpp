/**
 * サーバを介さないテストを行う。
 * 
 * e.g. compile.
 * g++ -O3 -DDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ test.cpp -o ../bin/test
*/
#include <iostream>
#include <cassert>
#include <nlohmann/json.hpp>
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

void json_sample_1() {
    puts("=== json_sample_1");
    nlohmann::json j;
    j["pi"] = 3.141;
    j["happy"] = true;
    j["name"] = "Niels";
    j["nothing"] = nullptr;
    j["answer"]["everything"] = 42;  // 存在しないキーを指定するとobjectが構築される
    j["list"] = { 1, 0, 2 };         // [1,0,2]
    j["object"] = { {"currency", "USD"}, {"value", 42.99} };  // {"currentcy": "USD", "value": 42.99}
    std::cout << j << std::endl;  // coutに渡せば出力できる。
}

void json_sample_2() {
    puts("=== json_sample_2");
    nlohmann::json j;
    j["personData"] = {{"name", "Jojo"}, {"email", "jojo@loki.org"}, {"age", 24}};
    std::cout << j << std::endl;
}

const char* personDataJsonSample() {
    return R"({
            "personData": {
                "name": "Jojo",
                "email": "jojo@loki.org",
                "age": 24
            }})";
}

/**
 * 設計・実装はここから
*/

using json = nlohmann::json;

template <class T>
class Controller {
public:
    virtual ~Controller() = default;
    virtual T execute() const = 0;
};

class CreatePersonCtl final : public Controller<json> {
public:
    static Controller<json>* factory(const std::string& uri) {
        if(uri == "/api/create/person/" || uri == "/hello_world/create/person") {
            return new CreatePersonCtl();
        }
        return nullptr;
    }
    virtual json execute() const override {
        puts("------ CreatePersonCtl::execute()");
        // TODO 実装
        return json();
    } 
};

int test_CreatePersonCtl() {
    puts("=== test_CreatePersonCtl");
    /**
     * 今回は std::unique_ptr の使いどころが難しいと考える。
     * まだ、学習中なので確かなことは言えないが、fcgi によって、エンドポイントのプロセスは起動し続ける。
     * つまり、std::unique_ptr の解放時期がおそらく見当たらない。よって、プログラマが正しく解放管理を行う
     * 必要があると思っている。
    */
    try {
        Controller<json>* ctl = CreatePersonCtl::factory("/api/foo/bar/");
        if(ctl) {            
        } else {
            puts("First ... mistake, no match URI.");
            ctl = CreatePersonCtl::factory("/api/create/person/");
            ctl->execute();
        }
        delete ctl;
        return EXIT_SUCCESS;
    } catch(std::exception& e) {
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
        json_sample_1();
        json_sample_2();
        ptr_api_debug<const char*, const char*>("Play and Result ... ", personDataJsonSample());
    }
    if(1.00) {
        auto ret = 0;
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_CreatePersonCtl());
        assert(ret == 0);
    }
    puts("END   Test === ");    
}