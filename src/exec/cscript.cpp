#include <boost/optional.hpp>
#include "exec/cscript.hpp"
#include "ftp/client.hpp"
#include "exec/util.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"

namespace exec
{

boost::optional<const cfg::setting::Cscript&> 
LookupCscript(const std::string& command, CscriptType type)
{
  for (const auto& cscript : cfg::Get().Cscript())
  {
    if (cscript.GetType() == type && cscript.Command() == command)
      return boost::optional<const cfg::setting::Cscript&>(cscript);
  }
  return boost::optional<const cfg::setting::Cscript&>();
}

bool Cscript(ftp::Client& client, const cfg::setting::Cscript& cscript,
      const std::string& fullCommand, CscriptType type, ftp::ReplyCode failCode)
{
  std::string group = client.User().PrimaryGID() != -1 ? 
                      acl::GroupCache::Group(client.User().PrimaryGID()).Name() :
                      "NoGroup";
                      
  util::ProcessReader::ArgvType argv =
  { cscript.Path().ToString(), fullCommand, client.User().Name(), group };
  
  try
  {
    exec::Reader reader(client, argv);
    std::string messages;
    
    try
    {
      std::string line;
      while (reader.Getline(line))
      {
        if (!messages.empty()) messages += '\n';
        messages += line;
      }
    }
    catch (const util::SystemError& e)
    {
      if (type == CscriptType::PRE)
        client.Control().Reply(failCode, 
              "Error while reading from pipe: " + e.Message());
      logs::error << "Error while reading from child process pipe: "
                  << boost::join(argv, " ") 
                  << ": " << e.Message() << logs::endl;
      return false;
    }
    
    reader.Close();
    
    if (type == CscriptType::POST)
    {
      if (!messages.empty())
      {
        logs::error << "Post cscript for command " << cscript.Command()
                    << " produced output which is being discarded." << logs::endl;
      }
      
      return reader.ExitStatus() == 0;
    }
    
    if (reader.ExitStatus() != 0)
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
  for (const auto& cscript : cfg::Get().Cscript())
  {
    if (cscript.GetType() == type && cscript.Command() == command)
    {
      if (!Cscript(client, cscript, fullCommand, type, failCode))
      {
        if (type == CscriptType::PRE) return false;
      }
    }
  }
  
  return true;
  auto cscript = LookupCscript(command, type);
  if (!cscript) return true;
}

} /* exec namespace */
