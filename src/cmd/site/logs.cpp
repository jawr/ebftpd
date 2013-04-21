#include <stack>
#include <boost/lexical_cast.hpp>
#include "cmd/site/logs.hpp"
#include "logs/logs.hpp"
#include "util/reverselogreader.hpp"
#include "cmd/error.hpp"
#include "util/path/path.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

LOGSCommand::LOGSCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
  Command(client, client.Control(), client.Data(), argStr, args),
  number(cfg::Get().LogLines())
{
}

bool LOGSCommand::ParseArgs()
{
  int n = 1;
  util::ToLower(args[n]);
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
    util::ToLower(args[n]);
  }
  
  log = args[n];
  strings.assign(args.begin() + n + 1, args.end());
  for (std::string& s : strings) util::ToLower(s);
  return true;
}

bool LOGSCommand::CheckStrings(std::string line)
{
  util::ToLower(line);
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
    control.Format(ftp::ActionNotOkay, "Unable to open log: %1%: %2%", path, e.Message());
    return;
  }
}

void LOGSCommand::Execute()
{ 
  if (!ParseArgs()) throw cmd::SyntaxError();

  const cfg::Config& config = cfg::Get();
  
  std::string filename;  
  if (log == "error") filename = config.ErrorLog().Name();
  else if (log == "security") filename = config.SecurityLog().Name();
  else if (log == "siteop") filename = config.SiteopLog().Name();
  else if (log == "events") filename = config.EventLog().Name();
  else if (log == "db") filename = config.DatabaseLog().Name();
  else if (log == "debug") filename = config.DebugLog().Name();
  else throw cmd::SyntaxError();
  
  Show(util::path::Join(config.Datapath(), "logs/" + filename + ".log"));
}

} /* site namespace */
} /* cmd namespace */
