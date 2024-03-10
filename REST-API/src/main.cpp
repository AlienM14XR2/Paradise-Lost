/**
 * REST API
 * 
 * fcgi を用いたエンドポイントに該当する。
 *  
 * e.g. compile.
 * g++ -O3 -DDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ main.cpp -lfcgi++ -lfcgi -o ../bin/endpoint
 * 
 * e.g. プロセス起動
 * spawn-fcgi -p 9900 -n endpoint
 * 
 * e.g. curl でアクセス
 * curl -i -s -X POST -d 'this is the post content.' http://localhost/api/create/person/
*/
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcgi_config.h>
#include <fcgi_stdio.h>

#include "rest_api_debug.hpp"

int main(void) {
    std::cout << "START REST API" << std::endl;
    try {
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