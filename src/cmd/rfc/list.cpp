#include "cmd/rfc/list.hpp"
#include "cfg/get.hpp"
#include "stats/util.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void LISTCommand::Execute()
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
  fs::Path path;
  if (args.size() >= 2)
  {
    std::string::size_type optOffset = 0;
    if (args[1][0] == '-')
    {
      options = args[1].substr(1);
      optOffset += args[1].length();
    }
    
    path = boost::trim_copy(std::string(argStr, optOffset));
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
    return;
  }
  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "End of directory listing (" + 
      stats::util::HighResSecondsString(data.State().StartTime(), data.State().EndTime()) + ")"); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
