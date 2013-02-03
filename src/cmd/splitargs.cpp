#include <boost/algorithm/string/trim.hpp>
#include "cmd/splitargs.hpp"

namespace cmd
{

bool SplitArgs(const std::string& command, std::vector<std::string>& args)
{
  args.clear();
  std::string::size_type pos1 = 0;
  std::string::size_type len = command.length();
  while (true)
  {
    while (std::isspace(command[pos1])) ++pos1;
    if (pos1 == len) break;
    
    std::string::size_type pos2 = command[pos1] == '{' ?
                                  command.find_first_of('}', pos1 + 1) :
                                  command.find_first_of(' ', pos1 + 1);
    if (pos2 == std::string::npos)
    {
      if (command[pos1] == '{') return false;
      args.emplace_back(command.substr(pos1));
      boost::trim(args.back());
      break;
    }
    else
    {
      if (command[pos1] == '{') ++pos1;
      args.emplace_back(command.substr(pos1, pos2 - pos1));
      boost::trim(args.back());
      pos1 = pos2 + 1;
    }
  }
  return true;
}

} /* cmd namespace */
