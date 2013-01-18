#include <boost/optional.hpp>
#include "exec/cscript.hpp"
#include "ftp/client.hpp"
#include "exec/util.hpp"
#include "logs/logs.hpp"
#include "exec/script.hpp"
#include "acl/groupcache.hpp"

namespace exec
{

bool Cscript(ftp::Client& client, const std::string& group, 
      const cfg::setting::Cscript& cscript, const std::string& fullCommand, 
      CscriptType type, ftp::ReplyCode failCode)
{                      
  util::ProcessReader::ArgvType argv =
  { cscript.Path().ToString(), fullCommand, client.User().Name(), group };
  
  try
  {
    std::string messages;
    int exitStatus = Script(client, argv, messages);
    
    if (type == CscriptType::POST)
    {
      if (!messages.empty())
      {
        logs::error << "Post cscript for command " << cscript.Command()
                    << " produced output which is being discarded." << logs::endl;
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
    logs::error << "Failed to execute cscript: " 
                << boost::join(argv, " ") 
                << ": " << e.Message() << logs::endl;
    return false;
  }
  
  return true;
}

bool Cscripts(ftp::Client& client, const std::string& command, 
      const std::string& fullCommand, CscriptType type, ftp::ReplyCode failCode)
{
  std::string group = acl::GroupCache::GIDToName(client.User().PrimaryGID());

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
