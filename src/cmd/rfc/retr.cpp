#include <ios>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/rfc/retr.hpp"
#include "fs/file.hpp"
#include "acl/usercache.hpp"
#include "db/stats/stat.hpp"
#include "stats/util.hpp"

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

  std::streamsize bytes = 0;
  try
  {
    char buffer[16384];
    while (true)
    {
      std::streamsize len = boost::iostreams::read(*fin,buffer, sizeof(buffer));
      if (len < 0) break;
      data.State().Update(len);
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
  
  data.Close();
  boost::posix_time::time_duration duration = data.State().EndTime() - data.State().StartTime();
  db::stats::Download(client.User(), bytes / 1024, duration.total_milliseconds());
  acl::UserCache::DecrCredits(client.User().Name(), bytes / 1024);

  control.Reply(ftp::DataClosedOkay, "Transfer finished @ " + 
      stats::util::AutoUnitSpeedString(stats::util::CalculateSpeed(data.State().Bytes(), duration))); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
