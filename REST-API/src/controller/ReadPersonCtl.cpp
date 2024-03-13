#include "../../inc/ReadPersonCtl.hpp"
#include <memory>
// ORM
#include "mysql/jdbc.h"
#include "rest_api_debug.hpp"
#include "ConnectionPool.hpp"
#include "DataField.hpp"
#include "PersonData.hpp"
#include "MySQLConnection.hpp"
#include "Repository.hpp"
#include "PersonRepository.hpp"
#include "RdbDataStrategy.hpp"
#include "PersonStrategy.hpp"
#include "RdbProcStrategy.hpp"
#include "MySQLReadStrategy.hpp"
#include "MySQLTx.hpp"


extern ConnectionPool<sql::Connection> app_cp;

Controller<nlohmann::json>* ReadPersonCtl::factory(const std::string& uri, const char* _json) 
{
    if(uri == "/api/read/person/" || uri == "/api/read/person") {
        return new ReadPersonCtl(app_cp.pop(), nlohmann::json::parse(_json));
    }
    return nullptr;
}
ReadPersonCtl::ReadPersonCtl(sql::Connection* _con, const nlohmann::json& _j): rawCon(_con), j(_j)
{}
ReadPersonCtl::~ReadPersonCtl()
{
    if(rawCon) {
        ptr_api_debug<const char*, const sql::Connection*>("rawCon addr is ", rawCon);
        app_cp.push(rawCon);
    }
}
nlohmann::json ReadPersonCtl::execute() const 
{
    puts("------ ReadPersonCtl::execute()");
    try {
        // 実装
        nlohmann::json result;
        std::cout << j << std::endl;

        std::unique_ptr<MySQLConnection>                    mcon = std::make_unique<MySQLConnection>(rawCon);
        std::unique_ptr<Repository<PersonData,std::size_t>> repo = std::make_unique<PersonRepository>(PersonRepository(mcon.get()));                
        for(auto v: j) {
            std::size_t id = v.at("id");
            std::unique_ptr<RdbProcStrategy<PersonData>> proc_strategy = std::make_unique<MySQLReadStrategy<PersonData,std::size_t>>(repo.get(), id);
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
        app_cp.push(rawCon);
        rawCon = nullptr;
        return result;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        throw std::runtime_error(e.what());
    }
}
