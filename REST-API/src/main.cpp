/**
 * REST API
 * 
 * fcgi を用いたエンドポイントに該当する。
 *  
 * e.g. compile.
 * ln -s ~/dev/c++/HandsOn/ORM-Cheshire/bin/ cheshire-bin
 * g++ -O3 -DNDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -c ./controller/CreatePersonCtl.cpp -o ../bin/CreatePersonCtl.o
 * g++ -O3 -DNDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -c ./controller/DeletePersonCtl.cpp -o ../bin/DeletePersonCtl.o
 * g++ -O3 -DNDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -c ./controller/ReadPersonCtl.cpp -o ../bin/ReadPersonCtl.o
 * g++ -O3 -DDEBUG -std=c++20 -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -L/usr/lib/x86_64-linux-gnu/ -pedantic-errors -Wall -Werror main.cpp -lmysqlcppconn -lmysqlcppconn8 -lfcgi++ -lfcgi ~/cheshire-bin/PersonRepository.o ~/cheshire-bin/sql_generator.o ~/cheshire-bin/PersonStrategy.o ~/cheshire-bin/PersonData.o ~/cheshire-bin/MySQLConnection.o ../bin/CreatePersonCtl.o ../bin/DeletePersonCtl.o ../bin/ReadPersonCtl.o -o ../bin/endpoint
 * 
 * e.g. プロセス起動
 * spawn-fcgi -p 9900 -n endpoint
 * 
 * e.g. curl でアクセス
 * curl -s -X POST -H "Content-Type: application/json; charset=UTF-8" -d '{"personData":{"id": 60}}' http://localhost/api/delete/person/
 * curl -s -X POST -H "Content-Type: application/json; charset=UTF-8" -d '{"personData":{"age":36,"email":"dodo@loki.org","name":"Dodo"}}' http://localhost/api/create/person/
 * 
*/
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <cassert>
#include <optional>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcgi_config.h>
#include <fcgi_stdio.h>
// REST
#include "rest_api_debug.hpp"
#include "Controller.hpp"
#include "CreatePersonCtl.hpp"
#include "DeletePersonCtl.hpp"
// ORM
#include "mysql/jdbc.h"
#include "ConnectionPool.hpp"

using json = nlohmann::json;


ConnectionPool<sql::Connection> app_cp;

void mysql_connection_pool(const std::string& server, const std::string& user, const std::string& password, const int& sum) 
{
    sql::Driver* driver = get_driver_instance();//MySQLDriver::getInstance().getDriver();
    for(int i=0; i<sum; i++) {
        sql::Connection* con = driver->connect(server, user, password);
        if(con->isValid()) {
            // puts("connected ... ");
            ptr_api_debug<const char*, const int&>("connected ... ", 0);
            con->setSchema("cheshire");
            // auto commit は true としておく、Tx が必要な場合はリポジトリで明確にすること。あるいは MySQLTx を利用すること。
            app_cp.push(con);
        } else {
            // puts("connection is invalid ... ");            
            ptr_api_debug<const char*, const int&>("connection is invalid ... ", 1);
        }
    }
}

/**
 * TODO
 * 
 * ORM でも自分の興味があることしかやっていない。
 * Validation は本当はとても重要な機能だが、全く実装していない。
 * 設計は行っておくべきと考える。
 * 
 * REST API において JSON でやり取りすることは、珍しくない。
 * RDBMS と REST 双方で利用される Data には std::string toJson() 
 * のようなメンバ関数があったほうが便利だと思う。
*/


const std::string rawJson{R"({
            "personData": {
                "id": 33,
                "name": "Jojo",
                "email": "jojo@loki.org",
                "age": 24
            }})"};


void action(Controller<json>* ctl, json* pret) {
    try {
        if(ctl) {
            *pret = ctl->execute();
            delete ctl;
            ctl = nullptr;
            std::cout << *pret << std::endl;
        }
    } catch(std::exception& e) {
        throw std::runtime_error(e.what());
    }
}


int main(void) {
    std::cout << "START REST API" << std::endl;
    try {
        mysql_connection_pool("tcp://127.0.0.1:3306", "derek", "derek1234", 10);
        // int count = 0;      // これが答えだったか、何らかのオブジェクトを Pool するならここで行い
        // json j = json::parse(rawJson);
        while(FCGI_Accept() >= 0)
        {
            // printf("content-type:text/html\r\n");
            // printf("\r\n");
            // printf("<title>Fast CGI Hello</title>");
            // printf("<h1>fast CGI hello</h1>");

            /**
             * JSON データ取得
            */
            char* contentLength = nullptr;
            char* buf = nullptr;
            contentLength = getenv("CONTENT_LENGTH");
            if(contentLength) {
                ptr_api_debug<const char*, const char*>("CONTENT_LENGTH: ", contentLength);
                auto size = atoll(contentLength);
                buf = (char*)malloc(size + 1);
                if(buf) {
                    for(auto i=0; i < size+1; i++) {
                        buf[i] = '\0';
                    }
                    auto r_size = FCGI_fread(buf, size, 1, FCGI_stdin);
                    ptr_api_debug<const char*, const decltype(r_size)&>("r_size: ", r_size);
                    ptr_api_debug<const char*, const char*>("buf: ", buf);
                }
            }

            json result;
            json* pret = &result;
            Controller<json>* ctl = nullptr;
            try {
                ctl = CreatePersonCtl::factory(getenv("REQUEST_URI"), buf);
                action(ctl, pret);
                ctl = DeletePersonCtl::factory(getenv("REQUEST_URI"), buf);
                action(ctl, pret);

                if(buf) {
                    free(buf);
                    buf = nullptr;
                }
                // if(ctl) {
                //     delete ctl; ctl = nullptr;
                // }
            } catch(std::exception& e) {
                result["error"] = {
                    {"message", e.what()}
                };                
                ptr_api_error<const decltype(e)&>(e);
                if(buf) {
                    free(buf);
                    buf = nullptr;
                }
                if(ctl) {
                    delete ctl;
                    ctl = nullptr;
                }
            }

            printf("Content-Type: application/json; charset=UTF-8\r\n");    // \r\n これと下のものがないと Bad Gateway になる。
            printf("\r\n");                                                 // \r\n 続けて書いても問題ないはず。
            if(!result.empty()) {
                printf("%s\n",result.dump().c_str());
            }
            /**
             * TODO Exception Handling
             * 
             * Exception のハンドリングが必要だね。
             * json に エラーメッセージを入れて返却できればいいと思う。
             * よくあるのは、エラーコードとエラーメッセージの組み合わせ
             * だけれど、今回はエラーメッセージだけでよい。
            */

            // std::cout << rawJson << std::endl;
            // printf("request uri is %s\n", getenv("REQUEST_URI"));   // これをもとに各処理に分岐できる、REST API のエンドポイントとして充分使えそう。
            // printf("Request number %d running on host<i>%s</i>\n",++count,getenv("SERVER_NAME"));            
        }
        // while を抜けた際に、取得したメモリは解放する
        puts("END   REST API ===");
        std::cout << "END REST API" << std::endl;
        return EXIT_SUCCESS;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        return EXIT_FAILURE;
    }
}