#include <ios>
#include "cmd/rfc/retr.hpp"
#include "fs/file.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace rfc
{

cmd::Result RETRCommand::Execute()
{
  fs::InStreamPtr fin;
  try
  {
    fin = fs::OpenFile(client, argStr);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay,
                 "Unable to open file: " + e.Message());
    return cmd::Result::Okay;
  }

  control.Reply(ftp::TransferStatusOkay,
               "Opening data connection for download of " + 
               fs::Path(argStr).Basename().ToString() + ".");

  try
  {
    data.Open(ftp::TransferType::Download);
  }
  catch (const util::net::NetworkError&e )
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " +
                 e.Message());
    return cmd::Result::Okay;
  }

  try
  {
    std::streamsize bytes(0);
    char buffer[16384];
    while (true)
    {
      std::streamsize len = boost::iostreams::read(*fin,buffer, sizeof(buffer));
      if (len < 0) break;
      bytes += len;
      data.Write(buffer, len);
    }
    bytes /= 1024;
    acl::UserCache::DecrCredits(client.User().Name(), (long long)bytes);
  }
  catch (const std::ios_base::failure&)
  {
    fin->close();
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                 "Error while reading from disk.");
    return cmd::Result::Okay;
  }
  catch (const util::net::NetworkError& e)
  {
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                 "Error while writing to data connection: " +
                 e.Message());
    return cmd::Result::Okay;
  }
  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "Transfer finished."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
