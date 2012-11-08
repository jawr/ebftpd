#include <ios>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/rfc/retr.hpp"
#include "fs/file.hpp"
#include "acl/usercache.hpp"
#include "db/stats/stat.hpp"
#include "stats/util.hpp"
#include "util/scopeguard.hpp"
#include "ftp/counter.hpp"
#include "ftp/util.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace rfc
{

cmd::Result RETRCommand::Execute()
{
  namespace pt = boost::posix_time;
  using util::scope_guard;
  using util::make_guard;
  
  if (!ftp::Counter::StartDownload(client.User().UID(), client.Profile().MaxSimDl()))
  {
    std::ostringstream os;
    os << "You have reached your maximum of " << client.Profile().MaxSimDl() 
       << " simultaenous download(s).";
    control.Reply(ftp::ActionNotOkay, os.str());
    return cmd::Result::Okay;    
  }
  
  scope_guard countGuard = make_guard([&]{ ftp::Counter::StopDownload(client.User().UID()); });  
  
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
    std::vector<char> asciiBuf;
    char buffer[16384];
    while (true)
    {
      std::streamsize len = boost::iostreams::read(*fin,buffer, sizeof(buffer));
      if (len < 0) break;
      
      data.State().Update(len);
      
      char *bufp = buffer;
      if (data.DataType() == ftp::DataType::ASCII)
      {
        ftp::util::ASCIITranscodeRETR(buffer, len, asciiBuf);
        len = asciiBuf.size();
        bufp = asciiBuf.data();
      }
      
      data.Write(bufp, len);

      if (client.Profile().MaxDlSpeed() > 0)
      {
        pt::time_duration elapsed = 
            pt::microsec_clock::local_time() - data.State().StartTime();
        pt::time_duration minElapsed = pt::microseconds((data.State().Bytes()  / 
            1024.0 / client.Profile().MaxDlSpeed()) * 1000000);
        boost::this_thread::sleep(minElapsed - elapsed);
      }
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
  pt::time_duration duration = data.State().EndTime() - data.State().StartTime();
  db::stats::Download(client.User(), bytes / 1024, duration.total_milliseconds());
  acl::UserCache::DecrCredits(client.User().Name(), bytes / 1024);

  control.Reply(ftp::DataClosedOkay, "Transfer finished @ " + 
      stats::util::AutoUnitSpeedString(stats::util::CalculateSpeed(data.State().Bytes(), duration))); 
  return cmd::Result::Okay;
  
  (void) countGuard;
}

} /* rfc namespace */
} /* cmd namespace */
