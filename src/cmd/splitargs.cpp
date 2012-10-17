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
    std::cout << command.substr(pos1) << std::endl;
    while (std::isspace(command[pos1])) ++pos1;
    if (pos1 == len) break;
    
    std::string::size_type pos2 = command[pos1] == '{' ?
                                  command.find_first_of('}', pos1 + 1) :
                                  command.find_first_of(' ', pos1 + 1);
    if (pos2 == std::string::npos)
    {
      if (command[pos1] == '{') return false;
      args.push_back(command.substr(pos1));
      break;
    }
    else
    {
      if (command[pos1] == '{') ++pos1;
      args.push_back(command.substr(pos1, pos2 - pos1));
      pos1 = pos2 + 1;
    }
  }
  return true;
}

} /* cmd namespace */

#ifdef CMD_SPLITARGS_TEST

#include <cassert>

int main(int argc, char** argv)
{
	using namespace cmd;
  
  std::string cmd = "help";
  std::vector<std::string> args;
  assert(SplitArgs(argv[1], args));
  std::cout << "hello" << std::endl;
  for (auto& arg : args) std::cout << arg << std::endl;
}

#endif
