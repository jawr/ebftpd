#include <ios>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/rfc/retr.hpp"
#include "fs/file.hpp"
#include "acl/usercache.hpp"
#include "db/stats/stats.hpp"

namespace cmd { namespace rfc
{

cmd::Result RETRCommand::Execute()
{
  namespace time = boost::posix_time;
  time::ptime start = time::microsec_clock::local_time();

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

  std::streamsize bytes(0);
  try
  {
    char buffer[16384];
    while (true)
    {
      std::streamsize len = boost::iostreams::read(*fin,buffer, sizeof(buffer));
      if (len < 0) break;
      bytes += len;
      data.Write(buffer, len);
    }
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
  
  bytes /= 1000;

  time::ptime end = time::microsec_clock::local_time();
  time::time_duration diff = end - start;

  db::stats::Download(client.User(), bytes, diff.total_milliseconds());
  
  acl::UserCache::DecrCredits(client.User().Name(), (long long)bytes);

  data.Close();
  control.Reply(ftp::DataClosedOkay, "Transfer finished."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
