#include <mongo/client/dbclient.h>
#include <boost/optional.hpp>
#include <list>
#include <csignal>
#include "db/replicator.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "db/error.hpp"
#include "util/verify.hpp"
#include "util/misc.hpp"

namespace db
{

namespace
{

class Tail
{
  std::string ns;
  mongo::BSONObj lastObj;
  boost::optional<mongo::BSONElement> lastOID;
  mongo::DBClientConnection& conn;
  std::auto_ptr<mongo::DBClientCursor> cursor;
  
  void SetLastOID(const mongo::BSONObj& obj)
  {
    lastObj = obj.copy();
    mongo::BSONElement oid;
    lastObj.getObjectID(oid);
    lastOID.reset(oid);
  }
  
  void InitialiseLastOID()
  {
    auto cursor = conn.query(ns, mongo::Query().sort(BSON("$natural" << -1)), 1);
    if (!cursor.get()) throw mongo::DBException("cursor error", 0);
    if (cursor->more()) SetLastOID(cursor->next());
  }
  
public:
  Tail(const std::string& ns, mongo::DBClientConnection& conn) : 
    ns(ns), conn(conn)
  {
    InitialiseLastOID();
  }
  
  mongo::BSONObj Next()
  {
    while (true)
    {
      if (!cursor.get())
      {
        mongo::Query query;
        if (lastOID) query = QUERY("_id" << BSON("$gt" << *lastOID));

        boost::this_thread::interruption_point();
        cursor = conn.query(ns, query.sort(BSON("$natural" << 1)), 0, 0, nullptr, 
                            mongo::QueryOption_CursorTailable | 
                            mongo::QueryOption_AwaitData);
        boost::this_thread::interruption_point();
      }
      
      while (true)
      {
        if (!cursor->more())
        {
          boost::this_thread::sleep(boost::posix_time::milliseconds(100));
          if (cursor->isDead())
          {
            cursor.reset();
            break;
          }
          
          continue;
        }
        
        auto entry = cursor->next();
        SetLastOID(entry);
        return entry;
      }
    }
  }
};

}

std::shared_ptr<Replicator> Replicator::instance;

void Replicator::LogFailed(const std::list<std::shared_ptr<Replicable>>& failed)
{
  std::ostringstream os;
  os << "Exceeded maximum retries while replicating caches: ";
  for (auto it = failed.begin(); it != failed.end(); ++it)
  {
    if (it != failed.begin()) os << ", ";
    os << (*it)->Collection();
  }
  logs::Database(os.str());
}

void Replicator::Replicate(const mongo::BSONObj& entry)
{
  try
  {
    std::string collection = entry["collection"].String();
    auto id = entry["id"];
    for (auto& cache : caches)
    {
      if (cache->Collection() == collection)
        cache->Replicate(id);
    }
  }
  catch (const mongo::DBException& e)
  {
    LogException("Replicate unserialize", e, entry);
  }
}

void Replicator::Populate()
{
  logs::Debug("Populating caches..");
  for (auto& cache : caches)
  {
    if (!cache->Populate())
    {
      logs::Database("Error while populating %1% cache.", cache->Collection());
    }
  }
}

void Replicator::Run()
{
  util::SetProcessTitle("DB REPLICATOR");
  mongo::DBClientConnection conn;
  bool first = true;
  while (true)
  {
    const auto& dbConfig = cfg::Get().Database();
    try
    {
      conn.connect(dbConfig.Host());
      if (dbConfig.NeedAuth())
      {
        std::string errmsg;
        if (!conn.auth(dbConfig.Name(), dbConfig.Login(), dbConfig.Password(), errmsg))
        {
          throw mongo::DBException(errmsg, 0);
        }
      }
    }
    catch (const mongo::DBException& e)
    {
      static const long connectRetryInterval = 15;
      LogException("Connect", e);
      boost::this_thread::sleep(boost::posix_time::seconds(connectRetryInterval));
      continue;
    }
    
    if (!first) Populate();
    else first = false;

    try
    {
      Tail tail(dbConfig.Name() + ".updatelog", conn);
      while (true)
      {
        auto entry = tail.Next();
        std::cout << entry << std::endl;
        Replicate(entry);
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Replication", e);
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
  }
}

bool Replicator::Register(const std::shared_ptr<Replicable>& cache)
{
  if (!cache->Populate()) return false;
  caches.emplace_back(cache);
  return true;
}

void Replicator::Start()
{
  verify(!thread.joinable());
  logs::Debug("Starting cache replication thread..");
  thread = boost::thread(&Replicator::Run, this);
}

void Replicator::Stop()
{
  if (thread.joinable())
  {
    logs::Debug("Stopping cache replication thread..");
    thread.interrupt();
    bool firstWait = true;
    while (!thread.timed_join(boost::posix_time::milliseconds(1000)))
    {
      if (firstWait)
      {
        logs::Debug("Waiting for replication thread..");
        firstWait = false;
      }
      else
        logs::Debug("Still waiting ror replication thread..");
    }
  }
}

} /* db namespace */
