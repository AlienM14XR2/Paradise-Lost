#ifndef _READPERSONCTL_H_
#define _READPERSONCTL_H_

#include "nlohmann/json.hpp"
#include "mysql/jdbc.h"
#include "Controller.hpp"

class ReadPersonCtl final : public Controller<nlohmann::json> {
public:
    static Controller<nlohmann::json>* factory(const std::string& uri, const char* _json);
    ReadPersonCtl(sql::Connection* _con, const nlohmann::json& _j);
    ~ReadPersonCtl();
    virtual nlohmann::json execute() const override;
private:
    mutable sql::Connection* rawCon = nullptr;
    mutable nlohmann::json j;
};

#endif
