#include <cassert>
#include <boost/thread/once.hpp>
#include "db/connection.hpp"
#include "cfg/get.hpp"
#include "db/error.hpp"

namespace db
{

boost::once_flag Connection::once = BOOST_ONCE_INIT;

void Connection::AuthenticateHook::onCreate(mongo::DBClientBase* conn)
{
  const auto& dbConfig = cfg::Get().Database();
  if (dbConfig.NeedAuth())
  {
    std::string errmsg;
    if (!conn->auth(dbConfig.Name(), dbConfig.Login(), dbConfig.Password(), errmsg))
    {
      throw db::DBError("Failed to authetnicate with the database.");
    }
  }
}

Connection::Connection(ConnectionMode mode) :
  mode(mode),
  database(cfg::Get().Database().Name())
{
  Create();
}

void Connection::Create()
{
  boost::call_once(&CreateAuthenticateHook, once);  
  
  try
  {
    scopedConn.reset(mongo::ScopedDbConnection::getScopedDbConnection(cfg::Get().Database().Host()));
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Failred to connect to database: " << e.what() << logs::endl;
    if (mode == ConnectionMode::Safe)
      throw e;
  }
  catch (const db::DBError& e)
  {
    logs::db << "Failed to connect to database: " << e.what() << logs::endl;
    if (mode == ConnectionMode::Safe)
      throw e;
  }
  
  if (mode == ConnectionMode::Fast)
    scopedConn->conn().setWriteConcern(mongo::W_NONE);
}

void Connection::CreateAuthenticateHook()
{
  mongo::pool.addHook(new AuthenticateHook());
}

} /* db namespace */
