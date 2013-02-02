#include <boost/regex.hpp>
#include <boost/thread/future.hpp>
#include "db/dupe/dupe.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/bson/bson.hpp"
#include "util/misc.hpp"

namespace db
{

typedef std::vector<mongo::BSONObj> QueryResults;

namespace dupe
{

void Add(const std::string& directory, const std::string& section)
{
  Pool::Queue(std::make_shared<db::Insert>("dupe", 
          BSON("directory" << directory << 
               "section" << section <<
               "nuked" << false)));
}

std::vector<DupeResult> Search(const std::vector<std::string>& terms, int limit)
{
  mongo::BSONObjBuilder bob;
  for (const std::string& term : terms)
  {
    bob.appendRegex("directory", util::EscapeRegex(term), "i");
  }
  
  mongo::Query query(bob.obj());
  QueryResults queryResults;
  boost::unique_future<bool> future;

  Pool::Queue(std::make_shared<db::Select>("dupe", query.sort("_id", -1),        
                    queryResults, future, limit));
  
  future.wait();
  
  std::vector<DupeResult> results;
  if (future.get())
  {
    for (const auto& obj : queryResults)
    {
      mongo::BSONElement oid;
      obj.getObjectID(oid);
      results.emplace_back(obj["directory"].String(),
                           obj["section"].String(),
                           db::bson::ToPosixTime(oid.OID().asDateT()));
    }
  }
  
  return results;
}

std::vector<DupeResult> Newest(int limit)
{
  return Search(std::vector<std::string>(), limit);
}

} /* dupe namespace */
} /* db namespace */
