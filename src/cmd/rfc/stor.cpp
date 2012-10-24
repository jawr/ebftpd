#include <ios>
#include "cmd/rfc/stor.hpp"
#include "fs/file.hpp"

namespace cmd { namespace rfc
{

cmd::Result STORCommand::Execute()
{
  fs::OutStreamPtr fout;
  try
  {
    fout = fs::CreateFile(client, argStr);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay,
                 "Unable to create file: " + e.Message());
    return cmd::Result::Okay;
  }

  control.Reply(ftp::TransferStatusOkay,
               "Opening data connection for upload of " +
               fs::Path(argStr).Basename().ToString() + ".");

  try
  {
    data.Open(ftp::TransferType::Upload);
  }
  catch (const util::net::NetworkError&e )
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return cmd::Result::Okay;
  }

  
  std::streamsize bytes(0);
  try
  {
    char buffer[16384];
    while (true)
    {
      size_t len = data.Read(buffer, sizeof(buffer));
      bytes += len;
    
      fout->write(buffer, len);
    }
  }
  catch (const util::net::EndOfStream&) { }
  catch (const util::net::NetworkError& e)
  {
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                 "Error while reading from data connection: " +
                 e.Message());
    return cmd::Result::Okay;
  }

  logs::debug << "BYTES: " << bytes << logs::endl;
  bytes *= client.UserProfile().Ratio();
  logs::debug << "BYTES: " << bytes << logs::endl;
  bytes /= 1000;
  logs::debug << "BYTES: " << bytes << logs::endl;
  acl::UserCache::IncrCredits(client.User().Name(), (long long)bytes);

  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "Transfer finished."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
