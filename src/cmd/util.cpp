#include <cctype>
#include "cmd/util.hpp"
#include "util/string.hpp"

namespace cmd
{

bool ParseCredits(std::string s, long long& credits)
{
  if (s.empty()) return false;
  
  std::string unit = "K";
  if (std::isalpha(s.back()))
  {
    unit.assign(s.end() - 1, s.end());
    s.erase(s.end() - 1, s.end());
    if (s.empty()) return false;
    util::ToUpper(unit);
  }
  
  try
  {
    credits = boost::lexical_cast<long long>(s);
    if (credits < 0) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    return false;
  }
  
  if (unit == "M") credits *= 1024;
  else if (unit == "G") credits *= 1024 * 1024;
  else if (unit != "K") return false;
  
  return true;
}

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
      util::Trim(args.back());
      break;
    }
    else
    {
      if (command[pos1] == '{') ++pos1;
      args.emplace_back(command.substr(pos1, pos2 - pos1));
      util::Trim(args.back());
      pos1 = pos2 + 1;
    }
  }
  return true;
}

} /* cmd namespace */
