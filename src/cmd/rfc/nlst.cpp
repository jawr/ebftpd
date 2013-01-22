#include "cmd/rfc/nlst.hpp"
#include "cfg/get.hpp"
#include "stats/util.hpp"

namespace cmd { namespace rfc
{

void NLSTCommand::Execute()
{
  std::ostringstream os;
  os << "Opening connection for directory listing";
  if (data.Protection()) os << " using TLS/SSL";
  os << ".";
  control.Reply(ftp::TransferStatusOkay, os.str());

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
  fs::VirtualPath path;
  if (args.size() >= 2)
  {
    std::string::size_type optOffset = 0;
    if (args[1][0] == '-')
    {
      options = args[1].substr(1);
      optOffset += args[1].length();
    }
    
    path = fs::PathFromUser(boost::trim_copy(argStr.substr(optOffset)));
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
  control.Reply(ftp::DataClosedOkay, "End of directory listing (" + 
      stats::HighResSecondsString(data.State().StartTime(), data.State().EndTime()) + ")"); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
