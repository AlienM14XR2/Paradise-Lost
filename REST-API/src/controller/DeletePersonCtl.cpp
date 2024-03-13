#include "../../inc/DeletePersonCtl.hpp"
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
#include "MySQLDeleteStrategy.hpp"
#include "MySQLTx.hpp"


extern ConnectionPool<sql::Connection> app_cp;

Controller<nlohmann::json>* DeletePersonCtl::factory(const std::string& uri, const char* _json) 
{
    if(uri == "/api/delete/person/" || uri == "/api/delete/person") {
        return new DeletePersonCtl(app_cp.pop(), nlohmann::json::parse(_json));
    }
    return nullptr;
}
DeletePersonCtl::DeletePersonCtl(sql::Connection* _con, const nlohmann::json& _j): rawCon(_con), j(_j)
{}
DeletePersonCtl::~DeletePersonCtl()
{
    if(rawCon) {
        ptr_api_debug<const char*, const sql::Connection*>("rawCon addr is ", rawCon);
        app_cp.push(rawCon);
    }
}
nlohmann::json DeletePersonCtl::execute() const 
{
    puts("------ DeletePersonCtl::execute()");
    try {
        // 実装
        nlohmann::json result;
        std::cout << j << std::endl;

        std::unique_ptr<MySQLConnection>                    mcon            = std::make_unique<MySQLConnection>(rawCon);
        std::unique_ptr<Repository<PersonData,std::size_t>> repo            = std::make_unique<PersonRepository>(PersonRepository(mcon.get()));
        for(auto v: j) {
            std::size_t id = v.at("id");
            std::unique_ptr<RdbProcStrategy<PersonData>>    proc_strategy_d = std::make_unique<MySQLDeleteStrategy<PersonData,std::size_t>>(repo.get(), id);
            MySQLTx tx(mcon.get(), proc_strategy_d.get());
            tx.executeTx();
            result["personData"] = {
                {"id", id}
            };
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
