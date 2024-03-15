/**
 * サーバを介さないテストを行う。
 * 
 * e.g. compile.
 * g++ -O3 -DNDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -c ./controller/CreatePersonCtl.cpp -o ../bin/CreatePersonCtl.o
 * g++ -O3 -DNDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -c ./controller/DeletePersonCtl.cpp -o ../bin/DeletePersonCtl.o
 * g++ -O3 -DNDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -c ./controller/ReadPersonCtl.cpp -o ../bin/ReadPersonCtl.o
 * g++ -O3 -DNDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -c ./controller/UpdatePersonCtl.cpp -o ../bin/UpdatePersonCtl.o
 * g++ -O3 -DDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -L/usr/lib/x86_64-linux-gnu/ test.cpp -lmysqlcppconn -lmysqlcppconn8 ~/cheshire-bin/PersonRepository.o ~/cheshire-bin/sql_generator.o ~/cheshire-bin/PersonStrategy.o ~/cheshire-bin/PersonData.o ~/cheshire-bin/MySQLConnection.o ../bin/CreatePersonCtl.o ../bin/DeletePersonCtl.o ../bin/ReadPersonCtl.o ../bin/UpdatePersonCtl.o -o ../bin/test
*/
#include <iostream>
#include <cassert>
#include <chrono>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>
// ORM
#include "mysql/jdbc.h"
#include "mysqlx/xdevapi.h"
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
#include "MySQLDeleteStrategy.hpp"
#include "MySQLReadStrategy.hpp"
#include "MySQLUpdateStrategy.hpp"
#include "MySQLTx.hpp"
// REST
#include "rest_api_debug.hpp"
#include "CreatePersonCtl.hpp"
#include "DeletePersonCtl.hpp"
#include "ReadPersonCtl.hpp"
#include "UpdatePersonCtl.hpp"

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







ConnectionPool<sql::Connection> app_cp;
ConnectionPool<mysqlx::Session> app_sp("mysqlx::Session.");

void mysql_connection_pool(const std::string& server, const std::string& user, const std::string& password, const int& sum) 
{
    sql::Driver* driver = get_driver_instance();//MySQLDriver::getInstance().getDriver();
    for(int i=0; i<sum; i++) {
        sql::Connection* con = driver->connect(server, user, password);
        if(con->isValid()) {
            ptr_api_debug<const char*, const int&>("connected ... ", 0);
            con->setSchema("cheshire");
            // auto commit は true としておく、Tx が必要な場合はリポジトリで明確にすること。あるいは MySQLTx を利用すること。
            app_cp.push(con);
        } else {
            ptr_api_debug<const char*, const int&>("connection is invalid ... ", 1);
        }
    }
}

void mysqlx_session_pool(const std::string& server, const int& port, const std::string& user, const std::string& passwd, const int& sum)
{
    puts("=== mysqlx_session_pool");
    for(int i=0; i<sum; i++) {
        puts("connected ... ");
        app_sp.push(new mysqlx::Session(server, port, user, passwd));
    }
}





using json = nlohmann::json;


int test_CreatePersonCtl(std::size_t* pid) {
    puts("=== test_CreatePersonCtl");
    /**
     * 今回は std::unique_ptr の使いどころが難しいと考える。
     * まだ、学習中なので確かなことは言えないが、fcgi によって、エンドポイントのプロセスは起動し続ける。
     * つまり、std::unique_ptr の解放時期がおそらく見当たらない。よって、プログラマが正しく解放管理を行う
     * 必要があると思っている。補足すると関数化してローカルで利用すれば、無論上記の話は該当しない。
    */
    try {
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
            for(auto v: result) {
                *pid = v.at("id");
            }
        }
        delete ctl;
        return EXIT_SUCCESS;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        return EXIT_FAILURE;
    }
}

int test_DeletePersonCtl(std::size_t* pid) {
    puts("=== test_DeletePersonCtl");
    try {
        // const char* cj = R"({"personData":{"id":24}})";
        std::string str(R"({"personData":{"id":)");
        str.append(" ").append(std::to_string(*pid)).append("}}");
        std::cout << str << std::endl;
        Controller<json>* ctl = DeletePersonCtl::factory("/api/delete/person/", str.c_str());
        json result = ctl->execute();
        std::cout << result << std::endl;
        delete ctl;
        return EXIT_SUCCESS;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        return EXIT_FAILURE;
    }
}

int test_ReadPersonCtl(std::size_t* pid) {
    puts("=== test_ReadPersonCtl");
    try {
        std::string str(R"({"personData":{"id":)");
        str.append(" ").append(std::to_string(*pid)).append("}}");
        std::cout << str << std::endl;
        Controller<json>* ctl = ReadPersonCtl::factory("/api/read/person/", str.c_str());
        json result = ctl->execute();
        std::cout << result << std::endl;
        delete ctl;
        return EXIT_SUCCESS;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        return EXIT_FAILURE;
    }
}

int test_UpdatePersonCtl(std::size_t* pid) {
    puts("=== test_UpdatePersonCtl");
    try {
        std::string str(R"({"personData":{"age":25,"email":"jojo2@loki.org","name":"JOJO","id":)");
        str.append(" ").append(std::to_string(*pid)).append("}}");
        std::cout << str << std::endl;
        Controller<json>* ctl = UpdatePersonCtl::factory("/api/update/person/", str.c_str());
        json result = ctl->execute();
        std::cout << result << std::endl;
        delete ctl;
        return EXIT_SUCCESS;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        return EXIT_FAILURE;
    }
}

int test_ctlx_CreatePersonCtl() {
    puts("=== test_ctlx_CreatePersonCtl");
    try {
        const char* cj = R"({"personData":{"age":21,"email":"fc3s@loki.org","name":"R スケ"}})";
        Controller<json>* ctl = ctlx::CreatePersonCtl::factory("/api/create/person/", cj);
        json result = ctl->execute();
        std::cout << result << std::endl;
        delete ctl;
        return EXIT_SUCCESS;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        return EXIT_FAILURE;
    }
}

int main(void) {
    puts("=== START Test");
    mysql_connection_pool("tcp://127.0.0.1:3306", "derek", "derek1234", 3);
    mysqlx_session_pool("localhost", 33060, "derek", "derek1234", 3);
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
        std::size_t id = 0;
        std::size_t* pid = &id;
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_CreatePersonCtl(pid));
        ptr_api_debug<const char*, const std::size_t&>("pid is ", *pid);
        assert(*pid != 0);
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_ReadPersonCtl(pid));
        assert(ret == 0);
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_UpdatePersonCtl(pid));
        assert(ret == 0);
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_DeletePersonCtl(pid));
        assert(*pid != 0);
    }
    if(1.01) {      // ormx 対応
        auto ret = 0;
        ptr_api_debug<const char*, const decltype(ret)&>("Play and Result ... ", ret = test_ctlx_CreatePersonCtl());
        assert(ret == 0);
    }
    puts("END   Test === ");    
}