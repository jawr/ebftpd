#include <ios>
#include <unistd.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/rfc/stor.hpp"
#include "fs/file.hpp"
#include "db/stats/stat.hpp"
#include "acl/usercache.hpp"
#include "stats/util.hpp"
#include "ftp/counter.hpp"
#include "util/scopeguard.hpp"
#include "ftp/util.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace rfc
{

cmd::Result STORCommand::Execute()
{
  namespace pt = boost::posix_time;

  using util::scope_guard;
  using util::make_guard;
  
  off_t offset = data.RestartOffset();
  if (offset > 0 && data.DataType() == ftp::DataType::ASCII)
  {
    control.Reply(ftp::BadCommandSequence, "Resume not supported on ASCII data type.");
    return cmd::Result::Okay;
  }

  if (!ftp::Counter::StartUpload(client.User().UID(), client.Profile().MaxSimUl()))
  {
    std::ostringstream os;
    os << "You have reached your maximum of " << client.Profile().MaxSimUl() 
       << " simultaenous upload(s).";
    control.Reply(ftp::ActionNotOkay, os.str());
    return cmd::Result::Okay;    
  }
  
  scope_guard countGuard = make_guard([&]{ ftp::Counter::StopUpload(client.User().UID()); });  

  fs::FileSinkPtr fout;
  try
  {
    if (data.RestartOffset() > 0)
      fout = fs::AppendFile(client, argStr, data.RestartOffset());
    else
      fout = fs::CreateFile(client, argStr);
  }
  catch (const util::SystemError& e)
  {
    std::ostringstream os;
    os << "Unable to " << (data.RestartOffset() > 0 ? "append" : "create")
       << " file: " << e.Message();
    control.Reply(ftp::ActionNotOkay, os.str());
    return cmd::Result::Okay;
  }
  
  std::stringstream os;
  os << "Opening " << (data.DataType() == ftp::DataType::ASCII ? "ASCII" : "BINARY") 
     << " connection for upload of " 
     << fs::Path(argStr).Basename().ToString();
  if (data.IsTLS()) os << " using TLS/SSL";
  os << ".";
  control.Reply(ftp::TransferStatusOkay, os.str());
  
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
  
  try
  {
    std::vector<char> asciiBuf;
    char buffer[16384];
    while (true)
    {
      size_t len = data.Read(buffer, sizeof(buffer));
      
      char *bufp  = buffer;
      if (data.DataType() == ftp::DataType::ASCII)
      {
        ftp::util::ASCIITranscodeSTOR(buffer, len, asciiBuf);
        len = asciiBuf.size();
        bufp = asciiBuf.data();
      }
      
      data.State().Update(len);
      
      fout->write(bufp, len);
      if (client.Profile().MaxUlSpeed() > 0)
        ftp::util::SpeedLimitSleep(data.State(), client.Profile().MaxUlSpeed());
    }
  }
  catch (const util::net::EndOfStream&) { }
  catch (const util::net::NetworkError& e)
  {
    fout->close();
    data.Close();
    fs::ForceDeleteFile(client, argStr);
    control.Reply(ftp::DataCloseAborted,
                 "Error while reading from data connection: " +
                 e.Message());
    return cmd::Result::Okay;
  }
  catch (const std::ios_base::failure& e)
  {
    fout->close();
    data.Close();
    fs::ForceDeleteFile(client, argStr);
    control.Reply(ftp::DataCloseAborted,
                  "Error while writing to disk: " + std::string(e.what()));
  }
  
  fout->close();
  data.Close();

  boost::posix_time::time_duration duration = data.State().EndTime() - data.State().StartTime();
  db::stats::Upload(client.User(), data.State().Bytes() / 1024, duration.total_milliseconds());

  long long credits = data.State().Bytes() * client.UserProfile().Ratio() / 1024;
  acl::UserCache::IncrCredits(client.User().Name(), credits);

  control.Reply(ftp::DataClosedOkay, "Transfer finished @ " + 
      stats::util::AutoUnitSpeedString(stats::util::CalculateSpeed(data.State().Bytes(), duration))); 

  return cmd::Result::Okay;
  
  (void) countGuard;
}

} /* rfc namespace */
} /* cmd namespace */
