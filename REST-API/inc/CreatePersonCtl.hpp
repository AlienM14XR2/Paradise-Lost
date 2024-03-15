#ifndef _CREATEPERSONCTL_H_
#define _CREATEPERSONCTL_H_

#include "Controller.hpp"
#include "nlohmann/json.hpp"
#include "mysql/jdbc.h"

class CreatePersonCtl final : public Controller<nlohmann::json> {
public:
    static Controller<nlohmann::json>* factory(const std::string& uri, const char* _json);
    CreatePersonCtl(sql::Connection* _con, const nlohmann::json& _j);
    ~CreatePersonCtl();
    virtual nlohmann::json execute() const override;
private:
    mutable sql::Connection* rawCon = nullptr;
    mutable nlohmann::json j;
};

/**
 * 以下
 * namespace ctlx
 * O/R Mapping において、mysqlx::Session を利用した データアクセスに対応した
 * コントローラー
*/

namespace ctlx {

}

#endif
