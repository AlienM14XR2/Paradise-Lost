#ifndef _CREATEPERSONCTL_H_
#define _CREATEPERSONCTL_H_

#include "nlohmann/json.hpp"
#include "Controller.hpp"

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

#endif
