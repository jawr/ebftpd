#include "cmd/rfc/nlst.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace rfc
{

void NLSTCommand::Execute()
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
    return;
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
    
    path = argStr.substr(optOffset);
    boost::trim(path);
  }
  
  DirectoryList dirList(client, data, path, ListOptions(options, ""),
                        cfg::Get().Lslong().MaxRecursion());

  try
  {
    dirList.Execute();    
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::DataCloseAborted,
                "Error whiling writing to data connection: " + e.Message());
  }
  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "End of directory listing."); 
}

} /* rfc namespace */
} /* cmd namespace */
