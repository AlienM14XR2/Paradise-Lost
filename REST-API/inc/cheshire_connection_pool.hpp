#ifndef _CHESHIRECONNECTIONPOOL_H_
#define _CHESHIRECONNECTIONPOOL_H_

namespace cheshire {

ConnectionPool<sql::Connection> app_cp;

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

}   // end namespace cheshire


#endif
