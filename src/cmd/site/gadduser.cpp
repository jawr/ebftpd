#include <sstream>
#include <boost/algorithm/string/split.hpp>
#include "cmd/site/gadduser.hpp"
#include "cmd/site/adduser.hpp"

namespace cmd { namespace site
{

void GADDUSERCommand::Execute()
{
  std::string cpArgStr("ADDUSER ");
  cpArgStr += args[2];
  for (auto it = args.begin() + 3; it != args.end(); ++it)
    cpArgStr += " " + *it;

  std::vector<std::string> cpArgs;
  boost::split(cpArgs, cpArgStr, boost::is_any_of(" "));
  ADDUSERCommand(client, cpArgStr, cpArgs).Execute(args[1]);
}

// end
}
} 

