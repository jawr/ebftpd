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

#include <boost/optional.hpp>
#include "util/string.hpp"
#include "exec/cscript.hpp"
#include "ftp/client.hpp"
#include "exec/util.hpp"
#include "logs/logs.hpp"
#include "exec/script.hpp"
#include "acl/user.hpp"
#include "ftp/control.hpp"

namespace exec
{

bool Cscript(ftp::Client& client, const std::string& group, 
      const cfg::Cscript& cscript, const std::string& fullCommand, 
      CscriptType type, ftp::ReplyCode failCode)
{                      
  util::ProcessReader::ArgvType argv =
  { cscript.Path(), fullCommand, client.User().Name(), group };
  
  try
  {
    std::string messages;
    int exitStatus = Script(client, argv, messages);
    
    if (type == CscriptType::Post)
    {
      if (!messages.empty())
      {
        logs::Error("Post cscript for command %1% produced output which is being discarded.",
                    cscript.Command());
      }
      
      return exitStatus == 0;
    }
    
    if (exitStatus != 0)
    {
      if (messages.empty())
        client.Control().Reply(failCode, "Command denied by pre cscript.");
      else
        client.Control().Reply(failCode, messages);
      return false;
    }

    if (!messages.empty())
      client.Control().PartReply(ftp::CodeDeferred, messages);
  }
  catch (const util::SystemError& e)
  {
    if (type == CscriptType::Pre)
      client.Control().Reply(failCode, "Unable to execute cscript: " + e.Message());
    logs::Error("Failed to execute cscript: %1%: %2%",
                util::Join(argv, " "), e.Message());
    return false;
  }
  
  return true;
}

bool Cscripts(ftp::Client& client, const std::string& command, 
      const std::string& fullCommand, CscriptType type, ftp::ReplyCode failCode)
{
  std::string group = client.User().PrimaryGroup();

  for (const auto& cscript : cfg::Get().Cscript())
  {
    if (cscript.GetType() == type && cscript.Command() == command)
    {
      if (!Cscript(client, group, cscript, fullCommand, type, failCode))
      {
        if (type == CscriptType::Pre) return false;
      }
    }
  }
  
  return true;
}

} /* exec namespace */
