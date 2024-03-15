#include "../../inc/CreatePersonCtl.hpp"
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
#include "MySQLCreateStrategy.hpp"
#include "MySQLTx.hpp"
#include "MySQLXTx.hpp"
#include "MySQLXCreateStrategy.hpp"


extern ConnectionPool<sql::Connection> app_cp;
extern ConnectionPool<mysqlx::Session> app_sp;

Controller<nlohmann::json>* CreatePersonCtl::factory(const std::string& uri, const char* _json)
{
    if(uri == "/api/create/person/" || uri == "/api/create/person") {
        return new CreatePersonCtl(app_cp.pop(), nlohmann::json::parse(_json));
    }
    return nullptr;
}
CreatePersonCtl::CreatePersonCtl(sql::Connection* _con, const nlohmann::json& _j): rawCon(_con), j(_j) 
{}
CreatePersonCtl::~CreatePersonCtl() 
{
    if(rawCon) {
        ptr_api_debug<const char*, const sql::Connection*>("rawCon addr is ", rawCon);
        app_cp.push(rawCon);
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
        for(auto v: j) {        // この実装は JSON から必要な値を取得する部分と、PersonData 構築、Tx 実行を分けるべきだと思う。
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
            break;
        }
        // 返却と初期化
        app_cp.push(rawCon);
        rawCon = nullptr;
        return result;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        throw std::runtime_error(e.what());
        // return nlohmann::json();
    }
}


/**
 * 以下
 * namespace ctlx
 * 定義
*/

Controller<nlohmann::json>* ctlx::CreatePersonCtl::factory(const std::string& uri, const char* _json)
{
    if(uri == "/api/create/person/" || uri == "/api/create/person") {
        return new ctlx::CreatePersonCtl(app_sp.pop(), nlohmann::json::parse(_json));
    }
    return nullptr;
}
ctlx::CreatePersonCtl::CreatePersonCtl(mysqlx::Session* _session, const nlohmann::json& _j): rawSession(_session), j(_j)
{}
ctlx::CreatePersonCtl::~CreatePersonCtl()
{
    if(rawSession) {
        ptr_api_debug<const char*, const mysqlx::Session*>("rawSession addr is ", rawSession);
        app_sp.push(rawSession);
    }
}
// ...
nlohmann::json ctlx::CreatePersonCtl::execute() const 
{
    puts("------ ctlx::CreatePersonCtl::execute()");
    try {
        // 実装
        nlohmann::json result;
        std::cout << j << std::endl;

        std::string name;
        std::string email;
        int age = -1;
        // JSON から必要な値を取得する
        for(auto v: j) {
            name = v.at("name");
            email = v.at("email");
            auto age_ = v.at("age");
            if(!age_.is_null()) {
                age = v.at("age");
            }
        }
        // ormx::PersonData 構築
        ormx::PersonData person = ormx::PersonData::dummy();
        if(age != -1) {
            person = ormx::PersonData(name, email, age);
        } else {
            person = ormx::PersonData(name, email);
        }
        // MySQLXTx の構築
        std::unique_ptr<Repository<ormx::PersonData, std::size_t>>        repo = std::make_unique<ormx::PersonRepository>(rawSession);
        std::unique_ptr<RdbProcStrategy<ormx::PersonData>>        procStrategy = std::make_unique<ormx::MySQLXCreateStrategy<ormx::PersonData, std::size_t>>(repo.get(), person);
        ormx::MySQLXTx tx(rawSession, procStrategy.get());
        std::optional<ormx::PersonData> after = tx.executeTx();
        // レスポンス用 JSON の作成
        if(after.has_value()) {
            if(after.value().getAge().has_value()) {
                result["personData"] = {
                    {"id", after.value().getId()}
                    ,{"name", after.value().getName()}
                    ,{"email", after.value().getEmail()}
                    ,{"age", after.value().getAge().value()}
                };
            } else {
                result["personData"] = {
                    {"id", after.value().getId()}
                    ,{"name", after.value().getName()}
                    ,{"email", after.value().getEmail()}
                };
            }
        }
        return result;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        throw std::runtime_error(e.what());
    }
}