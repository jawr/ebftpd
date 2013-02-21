#include <stack>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/logs.hpp"
#include "logs/logs.hpp"
#include "util/reverselogreader.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

bool LOGSCommand::ParseArgs()
{
  int n = 1;
  boost::to_lower(args[n]);
  if (args[n] == "-max")
  {
    if (args.size() < 4) return false;
    try
    {
      number = boost::lexical_cast<int>(args[n + 1]);
    }
    catch (const boost::bad_lexical_cast&)
    { return false; }
    if (number <= 0) return false;
    n += 2;
    boost::to_lower(args[n]);
  }
  
  log = args[n];
  strings.assign(args.begin() + n + 1, args.end());
  for (std::string& s : strings) boost::to_lower(s);
  return true;
}

bool LOGSCommand::CheckStrings(std::string line)
{
  boost::to_lower(line);
  for (auto& s : strings)
  {
    if (line.find(s) == std::string::npos)
      return false;
  }
  return true;
}

void LOGSCommand::Show(const std::string& path)
{
  try
  {
    util::ReverseLogReader in(path);
    std::stack<std::string> lines;
    std::string line;
    int count = 0;
    while (in.Getline(line) && count < number)
    {
      if (CheckStrings(line) && !line.empty())
      {
        lines.push(line);
        ++count;
      }
    }
    
    while (!lines.empty())
    {
      control.PartReply(ftp::CommandOkay, lines.top());
      lines.pop();
    }
  
    control.Reply(ftp::CommandOkay, "LOGS command finished");
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to open log: " + e.Message());
    return;
  }
}

void LOGSCommand::Execute()
{ 
  if (!ParseArgs()) throw cmd::SyntaxError();

  if (log == "error") Show(logs::error.Path());
  else if (log == "security") Show(logs::security.Path());
  else if (log == "siteop") Show(logs::siteop.Path());
  else if (log == "events") Show(logs::events.Path());
  else if (log == "db") Show(logs::db.Path());
  else if (log == "debug") Show(logs::debug.Path());
  else throw cmd::SyntaxError();
  
  return;
}

} /* site namespace */
} /* cmd namespace */
