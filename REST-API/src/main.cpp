/**
 * REST API
 * 
 * fcgi を用いたエンドポイントに該当する。
 *  
 * e.g. compile.
 * ln -s ~/dev/c++/HandsOn/ORM-Cheshire/bin/ cheshire-bin
 * g++ -O3 -DDEBUG -std=c++20 -I../inc/ -I/home/jack/dev/c++/HandsOn/ORM-Cheshire/inc/ -I/usr/include/mysql-cppconn-8/ -L/usr/lib/x86_64-linux-gnu/ -pedantic-errors -Wall -Werror main.cpp -lmysqlcppconn -lmysqlcppconn8 -lfcgi++ -lfcgi ~/cheshire-bin/PersonRepository.o ~/cheshire-bin/sql_generator.o ~/cheshire-bin/PersonStrategy.o ~/cheshire-bin/PersonData.o ~/cheshire-bin/MySQLConnection.o -o ../bin/endpoint
 * 
 * e.g. プロセス起動
 * spawn-fcgi -p 9900 -n endpoint
 * 
 * e.g. curl でアクセス
 * curl -i -s -X POST -d 'this is the post content.' http://localhost/api/create/person/
*/
#include <iostream>
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

#include "rest_api_debug.hpp"

// ORM
#include "mysql/jdbc.h"
#include "ConnectionPool.hpp"
// #include "MySQLDriver.hpp"   これを利用しようとすると上手くいかない。

namespace cheshire {

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

}   // end namespace cheshire


int main(void) {
    std::cout << "START REST API" << std::endl;
    try {
        cheshire::mysql_connection_pool("tcp://127.0.0.1:3306", "derek", "derek1234", 10);
        int count = 0;      // これが答えだったか、何らかのオブジェクトを Pool するならここで行い
        while(FCGI_Accept() >= 0)
        {
            printf("content-type:text/html\r\n");
            printf("\r\n");
            printf("<title>Fast CGI Hello</title>");
            printf("<h1>fast CGI hello</h1>");

            char* contentLength = nullptr;
            contentLength = getenv("CONTENT_LENGTH");
            if(contentLength) {
                ptr_api_debug<const char*, const char*>("CONTENT_LENGTH: ", contentLength);
                auto size = atoll(contentLength);
                char* buf = nullptr;
                buf = (char*)malloc(size + 1);
                if(buf) {
                    for(auto i=0; i < size+1; i++) {
                        buf[i] = '\0';
                    }
                    auto r_size = FCGI_fread(buf, size, 1, FCGI_stdin);
                    ptr_api_debug<const char*, const decltype(r_size)&>("r_size: ", r_size);
                    ptr_api_debug<const char*, const char*>("buf: ", buf);
                    free(buf);
                }
            }

            printf("request uri is %s\n", getenv("REQUEST_URI"));   // これをもとに各処理に分岐できる、REST API のエンドポイントとして充分使えそう。
            printf("Request number %d running on host<i>%s</i>\n",++count,getenv("SERVER_NAME"));
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