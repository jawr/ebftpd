//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <stack>
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
      number = util::StrToInt(args[n + 1]);
    }
    catch (const std::bad_cast&)
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
