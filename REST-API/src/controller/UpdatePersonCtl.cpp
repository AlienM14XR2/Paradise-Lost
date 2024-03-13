#include "../../inc/UpdatePersonCtl.hpp"
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
#include "MySQLUpdateStrategy.hpp"
#include "MySQLTx.hpp"


extern ConnectionPool<sql::Connection> app_cp;

Controller<nlohmann::json>* UpdatePersonCtl::factory(const std::string& uri, const char* _json) 
{
    if(uri == "/api/update/person/" || uri == "/api/update/person") {
        return new UpdatePersonCtl(app_cp.pop(), nlohmann::json::parse(_json));
    }
    return nullptr;
}
UpdatePersonCtl::UpdatePersonCtl(sql::Connection* _con, const nlohmann::json& _j): rawCon(_con), j(_j)
{}
UpdatePersonCtl::~UpdatePersonCtl()
{
    if(rawCon) {
        ptr_api_debug<const char*, const sql::Connection*>("rawCon addr is ", rawCon);
        app_cp.push(rawCon);
    }
}
nlohmann::json UpdatePersonCtl::execute() const 
{
    puts("------ UpdatePersonCtl::execute()");
    try {
        // TODO 実装
        nlohmann::json result;
        std::cout << j << std::endl;

        // JSON から必要な値を取得する
        std::size_t id_ = 0;
        std::string name_;
        std::string email_;
        int age_ = -1;
        for(auto v: j) {
            id_    = v.at("id");
            name_  = v.at("name");
            email_ = v.at("email");
            auto age = v.at("age");
            if(!age.is_null()) {
                age_ = v.at("age");
            }
        }
        DataField<std::size_t> id("id", id_);
        DataField<std::string> name("name", name_);
        DataField<std::string> email("email", email_);
        std::optional<DataField<int>> age = std::nullopt;
        if(age_ != -1) {
            age = DataField<int>("age", age_);
        }
        // PersonData の作成
        std::unique_ptr<RdbDataStrategy<PersonData>>        strategy = std::make_unique<PersonStrategy>();
        PersonData person(strategy.get(), id, name, email, age);
        std::unique_ptr<MySQLConnection>                    mcon = std::make_unique<MySQLConnection>(rawCon);
        std::unique_ptr<Repository<PersonData,std::size_t>> repo = std::make_unique<PersonRepository>(PersonRepository(mcon.get()));
        std::unique_ptr<RdbProcStrategy<PersonData>>        proc_strategy = std::make_unique<MySQLUpdateStrategy<PersonData,std::size_t>>(repo.get(), person);
        // Tx 実行
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
        // 返却と初期化
        app_cp.push(rawCon);
        rawCon = nullptr;
        return result;
    } catch(std::exception& e) {
        ptr_api_error<const decltype(e)&>(e);
        throw std::runtime_error(e.what());
    }
}
