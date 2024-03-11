/**
 * サーバを介さないテストを行う。
 * 
 * e.g. compile.
 * g++ -O3 -DDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -L/usr/lib/x86_64-linux-gnu/ test.cpp -lmysqlcppconn -lmysqlcppconn8 ~/cheshire-bin/PersonRepository.o ~/cheshire-bin/sql_generator.o ~/cheshire-bin/PersonStrategy.o ~/cheshire-bin/PersonData.o ~/cheshire-bin/MySQLConnection.o -o ../bin/test
*/
#include <iostream>
#include <cassert>
#include <chrono>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>
// ORM
#include "mysql/jdbc.h"
#include "ConnectionPool.hpp"
#include "DataField.hpp"
#include "PersonData.hpp"
#include "MySQLConnection.hpp"
#include "Repository.hpp"
#include "PersonRepository.hpp"
#include "RdbDataStrategy.hpp"
#include "PersonStrategy.hpp"
#include "RdbProcStrategy.hpp"
#include "MySQLCreateStrategy.hpp"
#include "MySQLTx.hpp"
// REST
#include "rest_api_debug.hpp"
#include "CreatePersonCtl.hpp"
#include "cheshire_connection_pool.hpp"

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

void json_parse_sample() {
    puts("=== json_parse_sample");
    const char* personData = personDataJsonSample();
    std::string str = personData;
    nlohmann::json j = nlohmann::json::parse(str);

    for(auto v: j) {
        ptr_api_debug<const char*, const decltype(v)&>("v is ", v);
        std::string name  = v.at("name");   // この name と 次の email の違い、とても重要でとても賢いと思った。     
        auto email = v.at("email");
        auto age   = v.at("age");
        ptr_api_debug<const char*, const decltype(name)&>("name is ", name);
        ptr_api_debug<const char*, const char*>("name type is ", typeid(name).name());
        ptr_api_debug<const char*, const decltype(email)&>("email is ", email);
        ptr_api_debug<const char*, const char*>("email type is ", typeid(email).name());
        ptr_api_debug<const char*, const decltype(age)&>("age is ", age);
    }
}

PersonData personDataSample() {
    DataField<std::size_t> id("id", 900ul);
    DataField<std::string> name("name", "Jojo");
    DataField<std::string> email("email", "jojo@loki.org");
    DataField<int> age("age", 24);
    PersonData jojo(nullptr,id,name,email,age);
    return jojo;
}

int test_mock_personData() {
    puts("=== test_mock_personData");
    try {
        auto start = std::chrono::system_clock::now();
        json_parse_sample();
        PersonData data = personDataSample();
        // personData to JSON
        nlohmann::json j;
        j["personData"] = {
            {"id", data.getId().getValue()}
            ,{"name", data.getName().getValue()}
            ,{"email", data.getEmail().getValue()}
            ,{"age", data.getAge().value().getValue()}
        };
        std::cout << j << std::endl;
        auto end = std::chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count(); //処理に要した時間をミリ秒に変換
        std::cout << "passed " << elapsed << " msec." << std::endl;
         return EXIT_SUCCESS;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        return EXIT_FAILURE;
    }
}

/**
 * 設計・実装はここから
*/

extern ConnectionPool<sql::Connection> cheshire::app_cp;


using json = nlohmann::json;



Controller<nlohmann::json>* CreatePersonCtl::factory(const std::string& uri, const char* _json)
{
    if(uri == "/api/create/person/" || uri == "/hello_world/create/person") {
        return new CreatePersonCtl(cheshire::app_cp.pop(), nlohmann::json::parse(_json));
    }
    return nullptr;
}
CreatePersonCtl::CreatePersonCtl(sql::Connection* _con, const nlohmann::json& _j): rawCon(_con), j(_j) 
{}
CreatePersonCtl::~CreatePersonCtl() 
{
    if(rawCon) {
        ptr_api_debug<const char*, const sql::Connection*>("rawCon addr is ", rawCon);
        cheshire::app_cp.push(rawCon);
    }
}
// ...
nlohmann::json CreatePersonCtl::execute() const 
{
    puts("------ CreatePersonCtl::execute()");
    try {
        // 実装
        nlohmann::json result;
        std::cout << j << std::endl;

        std::unique_ptr<MySQLConnection>                    mcon = std::make_unique<MySQLConnection>(rawCon);
        std::unique_ptr<Repository<PersonData,std::size_t>> repo = std::make_unique<PersonRepository>(PersonRepository(mcon.get()));                
        std::unique_ptr<RdbDataStrategy<PersonData>>        strategy = std::make_unique<PersonStrategy>();
        for(auto v: j) {
            std::string name_  = v.at("name");
            std::string email_ = v.at("email");
            DataField<std::string> name("name", name_);
            DataField<std::string> email("email", email_);
            std::optional<DataField<int>> age = std::nullopt;
            auto age_ = v.at("age");
            if(!age_.is_null()) {
                age = DataField<int>("age", age_);
            }
            PersonData person(strategy.get(), name, email, age);
            std::unique_ptr<RdbProcStrategy<PersonData>> proc_strategy = std::make_unique<MySQLCreateStrategy<PersonData,std::size_t>>(repo.get(), person);
            MySQLTx tx(mcon.get(), proc_strategy.get());
            std::optional<PersonData> after = tx.executeTx();
            if(after.has_value()) {
                    if(after.value().getAge().has_value()) {        // この仕組みは良くない、複数 option があった場合対応できない。
                        result["personData"] = {
                            {"id", after.value().getId().getValue()}
                            ,{"name", after.value().getName().getValue()}
                            ,{"email", after.value().getEmail().getValue()}
                            ,{"age", after.value().getAge().value().getValue()}
                        };
                    } else {
                        result["personData"] = {
                            {"id", after.value().getId().getValue()}
                            ,{"name", after.value().getName().getValue()}
                            ,{"email", after.value().getEmail().getValue()}
                        };
                }
            }
        }
        // 返却と初期化
        cheshire::app_cp.push(rawCon);
        rawCon = nullptr;
        return result;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        return json();
    }
}



int test_CreatePersonCtl() {
    puts("=== test_CreatePersonCtl");
    /**
     * 今回は std::unique_ptr の使いどころが難しいと考える。
     * まだ、学習中なので確かなことは言えないが、fcgi によって、エンドポイントのプロセスは起動し続ける。
     * つまり、std::unique_ptr の解放時期がおそらく見当たらない。よって、プログラマが正しく解放管理を行う
     * 必要があると思っている。
    */
    try {
        // sql::Connection* rawCon = nullptr;
        const char* cj = R"({"personData":{"age":24,"email":"jojo@loki.org","name":"Jojo"}})";
        Controller<json>* ctl = CreatePersonCtl::factory("/api/foo/bar/", cj);
        if(ctl) {            
        } else {
            puts("First ... mistake, no match URI.");
            // auto start = std::chrono::system_clock::now();
            std::clock_t start = clock();
            ctl = CreatePersonCtl::factory("/api/create/person/", cj);
            json result = ctl->execute();
            std::cout << result << std::endl;
            // auto end = std::chrono::system_clock::now();
            // double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count(); //処理に要した時間をミリ秒に変換
            // std::cout << "passed " << elapsed << " msec." << std::endl;
            std::clock_t end = clock();
            std::cout << "passed " << (double)(end-start)/CLOCKS_PER_SEC << " sec." << std::endl;
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
    cheshire::mysql_connection_pool("tcp://127.0.0.1:3306", "derek", "derek1234", 3);
    if(0.01) {
        auto ret = 0;
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_debug_and_error());
        assert(ret == 1);
        json_sample_1();
        json_sample_2();
        ptr_api_debug<const char*, const char*>("Play and Result ... ", personDataJsonSample());
        json_parse_sample();
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_mock_personData());
        assert(ret == 0);
    }
    if(1.00) {
        auto ret = 0;
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_CreatePersonCtl());
        assert(ret == 0);
    }
    puts("END   Test === ");    
}