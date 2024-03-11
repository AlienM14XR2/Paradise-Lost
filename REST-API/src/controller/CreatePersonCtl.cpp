#include "../../inc/CreatePersonCtl.hpp"
#include <memory>
#include "cheshire_connection_pool.hpp"
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


// extern ConnectionPool<sql::Connection> cheshire::app_cp;    // @see cheshire_connection_pool.hpp

