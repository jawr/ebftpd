#include <boost/optional.hpp>
#include "exec/check.hpp"
#include "fs/path.hpp"
#include "ftp/client.hpp"
#include "exec/script.hpp"
#include "fs/directory.hpp"
#include "util/string.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"
#include "ftp/control.hpp"
#include "acl/user.hpp"

namespace exec
{

boost::optional<const fs::Path>
LookupCheck(const std::vector<cfg::setting::CheckScript>& checks, const fs::VirtualPath& path)
{
  for (const auto& check : checks)
  {
    if (util::WildcardMatch(check.Mask(), path.ToString()))
    {
      if (check.Disabled()) break;
      return boost::optional<const fs::Path>(check.Path());
    }
  }
  
  return boost::optional<const fs::Path>();
}

bool PreCheck(ftp::Client& client, const fs::VirtualPath& path)
{
  auto scriptPath = LookupCheck(cfg::Get().PreCheck(), path);
  if (!scriptPath) return true;

  util::ProcessReader::ArgvType argv =
  {
    scriptPath->ToString(), 
    path.Basename().ToString(), 
    path.Dirname().ToString(), 
    fs::WorkDirectory().ToString()
  };
  
  try
  {
    std::string messages;
    int exitStatus = Script(client, argv, messages);
    
    if (exitStatus != 0)
    {
      if (messages.empty())
      {
        client.Control().Reply(ftp::BadFilename, "Upload denied by pre check script.");
      }
      else
      {
        client.Control().Reply(ftp::BadFilename, messages);
      }
      
      return false;
    }
    
    if (!messages.empty())
      client.Control().PartReply(ftp::CodeDeferred, messages);
  }
  catch (const util::SystemError& e)
  {
    client.Control().Reply(ftp::ActionNotOkay, "Unable to execute pre check script: " + e.Message());
    logs::Error("Unable to execute pre check script: %1%", e.Message());
    return false;
  }

  return true;
}

bool PreDirCheck(ftp::Client& client, const fs::VirtualPath& path)
{
  auto scriptPath = LookupCheck(cfg::Get().PreDirCheck(), path);
  if (!scriptPath) return true;

  util::ProcessReader::ArgvType argv =
  {
    scriptPath->ToString(), 
    path.Basename().ToString(), 
    path.Dirname().ToString(), 
    fs::WorkDirectory().ToString()
  };
  
  try
  {
    std::string messages;
    int exitStatus = Script(client, argv, messages);
    
    if (exitStatus != 0)
    {
      if (messages.empty())
      {
        client.Control().Reply(ftp::ActionNotOkay, "Mkdir denied by pre check script.");
      }
      else
      {
        client.Control().Reply(ftp::ActionNotOkay, messages);
      }
      
      return false;
    }
    
    if (!messages.empty())
      client.Control().PartReply(ftp::CodeDeferred, messages);
  }
  catch (const util::SystemError& e)
  {
    client.Control().Reply(ftp::ActionNotOkay, "Unable to execute pre dir check script: " + e.Message());
    logs::Error("Unable to execute pre dir check script: %1%", e.Message());
    return false;
  }

  return true;
}

bool PostCheck(ftp::Client& client, const fs::VirtualPath& path, 
      const std::string& crc, double speed, const std::string& section)
{
  auto scriptPath = LookupCheck(cfg::Get().PostCheck(), path);
  if (!scriptPath) return true;

  util::ProcessReader::ArgvType argv =
  {
    scriptPath->ToString(), 
    fs::MakeReal(path).ToString(), 
    crc,
    client.User().Name(),
    client.User().PrimaryGroup(),
    client.User().Tagline(),
    boost::lexical_cast<std::string>(static_cast<unsigned>(speed / 1024)),
    section
  };
  
  try
  {
    std::string messages;
    int exitStatus = Script(client, argv, messages);
    
    if (exitStatus != 0 && messages.empty())
      client.Control().PartReply(ftp::CodeDeferred, "Upload denied by post check script.");
    else
    if (!messages.empty())
      client.Control().PartReply(ftp::CodeDeferred, messages);
      
    return exitStatus == 0;
  }
  catch (const util::SystemError& e)
  {
    client.Control().Reply(ftp::ActionNotOkay, "Unable to execute post check script: " + e.Message());
    logs::Error("Unable to execute post check script: %1%", e.Message());
    return false;
  }
}

} /* exec namespace */
