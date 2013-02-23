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
      const cfg::setting::Cscript& cscript, const std::string& fullCommand, 
      CscriptType type, ftp::ReplyCode failCode)
{                      
  util::ProcessReader::ArgvType argv =
  { cscript.Path(), fullCommand, client.User().Name(), group };
  
  try
  {
    std::string messages;
    int exitStatus = Script(client, argv, messages);
    
    if (type == CscriptType::POST)
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
    if (type == CscriptType::PRE)
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
        if (type == CscriptType::PRE) return false;
      }
    }
  }
  
  return true;
}

} /* exec namespace */
