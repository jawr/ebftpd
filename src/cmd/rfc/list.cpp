#include "cmd/rfc/list.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace rfc
{

cmd::Result LISTCommand::Execute()
{
  control.Reply(ftp::TransferStatusOkay,
               "Opening data connection for directory listing.");

  try
  {
    data.Open(ftp::TransferType::List);
  }
  catch (const util::net::NetworkError&e )
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return cmd::Result::Okay;
  }
  
  std::string options;
  std::string path;
  if (args.size() >= 2)
  {
    std::string::size_type optOffset = 0;
    if (args[1][0] == '-')
    {
      options = args[1].substr(1);
      optOffset += args[1].length();
    }
    
    path = std::string(argStr, optOffset);
    boost::trim(path);
  }

  
  const cfg::Config& config = cfg::Get();
  std::string forcedOptions = "l" + config.Lslong().Options();
  
  DirectoryList dirList(client, data, path, ListOptions(options, forcedOptions),
                        config.Lslong().MaxRecursion());

  try
  {
    dirList.Execute();    
  }
  catch (const util::net::NetworkError& e)
  {
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                "Error whiling writing to data connection: " + e.Message());
    return cmd::Result::Okay;
  }
  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "End of directory listing."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
