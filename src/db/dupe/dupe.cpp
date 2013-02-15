#include <boost/regex.hpp>
#include <boost/thread/future.hpp>
#include "db/dupe/dupe.hpp"
#include "db/connection.hpp"
#include "db/serialization.hpp"
#include "util/misc.hpp"

namespace db
{

typedef std::vector<mongo::BSONObj> QueryResults;

namespace dupe
{

DupeResult Unserialize(const mongo::BSONObj& obj)
{
  try
  {
    mongo::BSONElement oid;
    obj.getObjectID(oid);
    return DupeResult(obj["directory"].String(),
                      obj["section"].String(),
                      db::ToPosixTime(oid.OID().asDateT()));
  }
  catch (const mongo::DBException& e)
  {
    LogException("Dupe result unserialize", e, obj);
    throw e;
  }
}

void Add(const std::string& directory, const std::string& section)
{
  FastConnection conn;
  conn.Insert("dupe", BSON("directory" << directory << 
                           "section" << section <<
                           "nuked" << false));
}

std::vector<DupeResult> Search(const std::vector<std::string>& terms, int limit)
{
  mongo::BSONObjBuilder bob;
  for (const std::string& term : terms)
  {
    bob.appendRegex("directory", util::EscapeRegex(term), "i");
  }
  
  mongo::Query query(bob.obj());
  
  NoErrorConnection conn;
  return conn.QueryMulti<DupeResult>("dupe", query, limit);
}

std::vector<DupeResult> Newest(int limit)
{
  return Search(std::vector<std::string>(), limit);
}

} /* dupe namespace */
} /* db namespace */
