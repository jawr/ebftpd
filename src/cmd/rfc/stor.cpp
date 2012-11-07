#include <ios>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/rfc/stor.hpp"
#include "fs/file.hpp"
#include "db/stats/stat.hpp"
#include "acl/usercache.hpp"
#include "stats/util.hpp"
#include "ftp/counter.hpp"
#include "util/scopeguard.hpp"

namespace cmd { namespace rfc
{

cmd::Result STORCommand::Execute()
{
  namespace pt = boost::posix_time;

  using util::scope_guard;
  using util::make_guard;
  
  if (!ftp::Counter::StartUpload(client.User().UID(), client.Profile().MaxSimUl()))
  {
    std::ostringstream os;
    os << "You have reached your maximum of " << client.Profile().MaxSimUl() 
       << " simultaenous upload(s).";
    control.Reply(ftp::ActionNotOkay, os.str());
    return cmd::Result::Okay;    
  }
  
  scope_guard countGuard = make_guard([&]{ ftp::Counter::StopUpload(client.User().UID()); });  

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
  
  try
  {
    char buffer[16384];
    while (true)
    {
      size_t len = data.Read(buffer, sizeof(buffer));
      data.State().Update(len);
      fout->write(buffer, len);
      
      if (client.Profile().MaxUlSpeed() > 0)
      {
        pt::time_duration elapsed = 
            pt::microsec_clock::local_time() - data.State().StartTime();
        pt::time_duration minElapsed = pt::microseconds((data.State().Bytes()  / 
            1024.0 / client.Profile().MaxUlSpeed()) * 1000000);
        boost::this_thread::sleep(minElapsed - elapsed);
      }
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
