#include <ios>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/rfc/stor.hpp"
#include "fs/file.hpp"
#include "db/stats/stat.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace rfc
{

cmd::Result STORCommand::Execute()
{
  namespace time = boost::posix_time;
  time::ptime start = time::microsec_clock::local_time();

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

  time::ptime end = time::microsec_clock::local_time();
  time::time_duration diff = end - start;

  db::stats::Upload(client.User(), bytes/1000, diff.total_milliseconds());

  bytes *= client.UserProfile().Ratio();
  bytes /= 1000;
  acl::UserCache::IncrCredits(client.User().Name(), (long long)bytes);

  data.Close();
  control.Reply(ftp::DataClosedOkay, "Transfer finished."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
