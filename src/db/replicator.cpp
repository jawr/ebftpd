//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <boost/optional.hpp>
#include <list>
#include <csignal>
#include "db/replicator.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "db/error.hpp"
#include "util/verify.hpp"
#include "util/misc.hpp"
#include "db/connection.hpp"

namespace db
{

namespace
{

class Tail
{
  std::string ns;
  mongo::BSONObj lastObj;
  boost::optional<mongo::BSONElement> lastOID;
  mongo::DBClientBase& conn;
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
  Tail(const std::string& ns, mongo::DBClientBase& conn) : 
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
        
        {
          boost::this_thread::disable_interruption noInterrupt;
          cursor = conn.query(ns, query.sort(BSON("$natural" << 1)), 0, 0, nullptr, 
                              mongo::QueryOption_CursorTailable | 
                              mongo::QueryOption_AwaitData);
        }
        
        boost::this_thread::interruption_point();
      }
      
      while (true)
      {
        {
          boost::this_thread::disable_interruption noInterrupt;
          if (!cursor->more())
          {
            boost::this_thread::restore_interruption restoreInterrupt(noInterrupt);
            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            if (cursor->isDead())
            {
              cursor.reset();
              break;
            }
            
            continue;
          }
        }
        
        auto entry = cursor->next();
        SetLastOID(entry);
        return entry;
      }
    }
  }
};

}

std::unique_ptr<Replicator> Replicator::instance;

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
  util::SetProcessTitle("REPLICATOR");
  logs::SetThreadIDPrefix('R' /* replicator */);

  while (true)
  {
    try
    {
      SafeConnection conn;    
      Populate();

      try
      {
        Tail tail(cfg::Get().Database().Name() + ".updatelog", conn.BaseConn());
        while (true)
        {
          auto entry = tail.Next();
          Replicate(entry);
        }
      }
      catch (const mongo::DBException& e)
      {
        LogException("Replication", e);
      }      
    }
    catch (const DBError&)
    {
      // failed connection
    }

    boost::this_thread::sleep(boost::posix_time::seconds(retryInterval));
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
