#include <sstream>
#include "cmd/site/search.hpp"
#include "db/index/index.hpp"

namespace cmd { namespace site
{

void SEARCHCommand::Execute()
{
  auto results = db::index::Search(std::vector<std::string>(args.begin() + 1, args.end()));
  if (results.empty()) control.Reply(ftp::CommandOkay, "No search results.");
  else
  {
    std::ostringstream os;
    for (const auto& result : results)
    {
      os << result.dateTime << ": " << result.path << "\n";
    }
    control.Reply(ftp::CommandOkay, os.str());
  }
}

} /* site namespace */
} /* cmd namespace */
