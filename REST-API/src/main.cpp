/**
 * REST API
 * 
 * fcgi を用いたエンドポイントに該当する。
 * 
 * e.g. compile.
 * g++ -O3 -DDEBUG -std=c++20 -pedantic-errors -Wall -Werror -I../inc/ main.cpp -lfcgi++ -lfcgi -o ../bin/api
*/
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcgi_config.h>
#include <fcgi_stdio.h>


int main(void) {
    puts("=== START REST API");
    int count = 0;      // これが答えだったか、何らかのオブジェクトを Pool するならここで行い
    while(FCGI_Accept() >= 0)
    {
        printf("content-type:text/html\r\n");
        printf("\r\n");
        printf("<title>Fast CGI Hello</title>");
        printf("<h1>fast CGI hello</h1>");

        printf("request uri is %s\n", getenv("REQUEST_URI"));   // これをもとに各処理に分岐できる、REST API のエンドポイントとして充分使えそう。
        printf("Request number %d running on host<i>%s</i>\n",++count,getenv("SERVER_NAME"));
    }
    // while を抜けた際に、取得したメモリは解放する
    puts("END   REST API ===");
    return 0;
}