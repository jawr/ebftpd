#include <boost/regex.hpp>
#include <boost/thread/future.hpp>
#include "db/index/index.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/bson/bson.hpp"

namespace db { namespace index
{

void Add(const std::string& path)
{
  Pool::Queue(std::make_shared<db::Insert>("index", 
            BSON("path" << path << "date time" << mongo::Date_t())));
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
    bob.appendRegex("path", "/" + EscapeSearchTerm(term) + "/", "i");
  }
  
  QueryResults queryResults;
  boost::unique_future<bool> future;
  Pool::Queue(std::make_shared<db::Select>("index", bob.obj(), queryResults, future));
  
  future.wait();
  
  std::vector<SearchResult> results;
  if (future.get())
  {
    for (const auto& obj : queryResults)
    {
      results.emplace_back(obj["path"].String(),
                           db::bson::ToPosixTime(obj["date time"].Date()));
    }
  }
  
  return results;
}

} /* index namespace */
} /* db namespace */
