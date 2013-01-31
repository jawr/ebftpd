#include <boost/regex.hpp>
#include <boost/thread/future.hpp>
#include "db/index/index.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/bson/bson.hpp"

namespace db { namespace index
{

void Add(const std::string& path, const std::string& section)
{
  Pool::Queue(std::make_shared<db::Insert>("index", 
          BSON("path" << path << "section" << section)));
}

void Delete(const std::string& path)
{
  Pool::Queue(std::make_shared<db::Delete>("index", QUERY("path" << path)));
}

std::string EscapeSearchTerm(const std::string& term)
{
  boost::regex esc("[\\^\\.\\$\\|\\(\\)\\[\\]\\*\\+\\?\\/\\\\]");
  std::string rep("\\\\\\1");
  return boost::regex_replace(term, esc, rep, boost::match_default | boost::format_sed);
}

std::vector<SearchResult> Search(const std::vector<std::string>& terms)
{
  mongo::BSONObjBuilder bob;
  for (const std::string& term : terms)
  {
    bob.appendRegex("path", EscapeSearchTerm(term), "i");
  }
  
  mongo::Query query(bob.obj());
  QueryResults queryResults;
  boost::unique_future<bool> future;

  Pool::Queue(std::make_shared<db::Select>("index", query, queryResults, future));
  
  future.wait();
  
  std::vector<SearchResult> results;
  if (future.get())
  {
    for (const auto& obj : queryResults)
    {
      mongo::BSONElement oid;
      obj.getObjectID(oid);
      results.emplace_back(obj["path"].String(),
                           obj["section"].String(),
                           db::bson::ToPosixTime(oid.OID().asDateT()));
    }
  }
  
  return results;
}

} /* index namespace */
} /* db namespace */
