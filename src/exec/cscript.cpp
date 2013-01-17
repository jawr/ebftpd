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

bool Cscript(ftp::Client& client, const std::string& command, 
      const std::string& fullCommand, CscriptType type)
{
  auto cscript = LookupCscript(command, type);
  if (!cscript) return true;

  util::ProcessReader::ArgvType argv =
  { cscript->Path().ToString(), fullCommand, client.User().Name(), "GROUP" };

  std::cout << (type == CscriptType::PRE ? "PRE" : "POST") << std::endl;
  
  try
  {
    exec::Reader reader(client, argv);
    std::string messages;
    
    try
    {
      std::string buffer;
      while (reader.Read(buffer))
      {
        messages += buffer;
      }
    }
    catch (const util::SystemError& e)
    {
      if (type == CscriptType::PRE)
        client.Control().Reply(ftp::ActionNotOkay, 
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
        logs::error << "Post cscript for command " << command 
                    << " produced output which is being discarded." << logs::endl;
      }
      
      return reader.ExitStatus() == 0;
    }
    
    if (reader.ExitStatus() != 0)
    {
      if (messages.empty())
        client.Control().Reply(ftp::ActionNotOkay, "Command denied by pre cscript.");
      else
        client.Control().Reply(ftp::ActionNotOkay, messages);
      return false;
    }

    if (!messages.empty())
      client.Control().PartReply(ftp::CodeDeferred, messages);
  }
  catch (const util::SystemError& e)
  {
    if (type == CscriptType::PRE)
      client.Control().PartReply(ftp::ActionNotOkay, "Unable to execute cscript: " + e.Message());
    logs::error << "Failed to execute cscript: " 
                << boost::join(argv, " ") 
                << ": " << e.Message() << logs::endl;
    return false;
  }
  
  return true;
}

} /* exec namespace */
