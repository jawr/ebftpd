#include <stack>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/logs.hpp"
#include "logs/logs.hpp"
#include "util/reverselogreader.hpp"
#include "cmd/error.hpp"
#include "util/path/path.hpp"
#include "cfg/get.hpp"

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

  const cfg::Config& config = cfg::Get();
  
  std::string filename;  
  if (log == "error") filename = config.ErrorLog().Filename();
  else if (log == "security") filename = config.ErrorLog().Filename();
  else if (log == "siteop") filename = config.SiteopLog().Filename();
  else if (log == "events") filename = config.EventLog().Filename();
  else if (log == "db") filename = config.DatabaseLog().Filename();
  else if (log == "debug") filename = config.DebugLog().Filename();
  else throw cmd::SyntaxError();
  
  filename += ".log";  
  Show(util::path::Join(config.Datapath(), filename));
}

} /* site namespace */
} /* cmd namespace */
