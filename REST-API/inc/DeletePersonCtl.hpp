#ifndef _DELETEPERSONCTL_H_
#define _DELETEPERSONCTL_H_

#include "nlohmann/json.hpp"
#include "mysql/jdbc.h"
#include "Controller.hpp"

class DeletePersonCtl final : public Controller<nlohmann::json> {
public:
    static Controller<nlohmann::json>* factory(const std::string& uri, const char* _json);
    DeletePersonCtl(sql::Connection* _con, const nlohmann::json& _j);
    ~DeletePersonCtl();
    virtual nlohmann::json execute() const override;
private:
    mutable sql::Connection* rawCon = nullptr;
    mutable nlohmann::json j;
};

#endif
