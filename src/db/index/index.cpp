#include <boost/regex.hpp>
#include <boost/thread/future.hpp>
#include "db/index/index.hpp"
#include "util/misc.hpp"
#include "db/connection.hpp"

namespace db
{

typedef std::vector<mongo::BSONObj> QueryResults;

namespace index
{


SearchResult Unserialize(const mongo::BSONObj& obj)
{
  mongo::BSONElement oid;
  obj.getObjectID(oid);
  return SearchResult(obj["path"].String(),
                      obj["section"].String(),
                      ToPosixTime(oid.OID().asDateT()));
}

void Add(const std::string& path, const std::string& section)
{
  FastConnection conn;
  conn.Insert("index", BSON("path" << path << "section" << section));
}

void Delete(const std::string& path)
{
  NoErrorConnection conn;
  conn.Remove("index", QUERY("path" << path));
}

std::vector<SearchResult> Search(const std::vector<std::string>& terms, int limit)
{
  mongo::BSONObjBuilder bob;
  for (const std::string& term : terms)
  {
    bob.appendRegex("path", util::EscapeRegex(term), "i");
  }
  
  mongo::Query query(bob.obj());
  NoErrorConnection conn;
  return conn.QueryMulti<SearchResult>("index", query.sort("_id", -1), limit);
}

std::vector<SearchResult> Newest(int limit)
{
  return Search(std::vector<std::string>(), limit);
}

} /* index namespace */
} /* db namespace */
