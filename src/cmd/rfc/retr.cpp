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
#include "cfg/get.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace rfc
{

void RETRCommand::Execute()
{
  namespace pt = boost::posix_time;
  using util::scope_guard;
  using util::make_guard;
  
  off_t offset = data.RestartOffset();
  if (offset > 0 && data.DataType() == ftp::DataType::ASCII)
  {
    control.Reply(ftp::BadCommandSequence, "Resume not supported on ASCII data type.");
    throw cmd::NoPostScriptError();
  }
  
  if (!ftp::Counter::StartDownload(client.User().UID(), client.Profile().MaxSimDl()))
  {
    std::ostringstream os;
    os << "You have reached your maximum of " << client.Profile().MaxSimDl() 
       << " simultaenous download(s).";
    control.Reply(ftp::ActionNotOkay, os.str());
    throw cmd::NoPostScriptError();
  }
  
  scope_guard countGuard = make_guard([&]{ ftp::Counter::StopDownload(client.User().UID()); });  
  
  fs::VirtualPath path(fs::PathFromUser(argStr));

  fs::FileSourcePtr fin;
  try
  {
    fin = fs::OpenFile(client, path);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay,
                 "Unable to open file: " + e.Message());
    throw cmd::NoPostScriptError();
  }

  off_t size;
  try
  {
    size = fin->seek(0, std::ios_base::end) - std::streampos(0);  
    if (offset > size)
    {
      control.Reply(ftp::InvalidRESTParameter, "Restart offset larger than file size.");
      throw cmd::NoPostScriptError();
    }

    fin->seek(offset, std::ios_base::beg);
  }
  catch (const std::ios_base::failure& e)
  {
    std::string errmsg = e.what();
    errmsg[0] = std::toupper(errmsg[0]);
    control.Reply(ftp::ActionAbortedError, errmsg);
  }
  
  if (data.DataType() == ftp::DataType::ASCII &&
      !cfg::Get().AsciiDownloads().Allowed(size, path.Basename().ToString()))
  {
    control.Reply(ftp::ActionNotOkay, "File can't be downloaded in ASCII, change to BINARY.");
    throw cmd::NoPostScriptError();
  }

  std::stringstream os;
  os << "Opening " << (data.DataType() == ftp::DataType::ASCII ? "ASCII" : "BINARY") 
     << " connection for download of " 
     << fs::MakePretty(path).ToString()
     << " (" << size << " bytes)";
  if (data.Protection()) os << " using TLS/SSL";
  os << ".";
  control.Reply(ftp::TransferStatusOkay, os.str());

  try
  {
    data.Open(ftp::TransferType::Download);
  }
  catch (const util::net::NetworkError&e )
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " +
                 e.Message());
    throw cmd::NoPostScriptError();
  }

  std::streamsize bytes = 0;
  try
  {
    bool dlIncomplete = cfg::Get().DlIncomplete();
    std::vector<char> asciiBuf;
    char buffer[16384];
    while (true)
    {
      std::streamsize len = fin->read(buffer, sizeof(buffer));
      if (len < 0) 
      {
        if (!dlIncomplete || !fs::IsIncomplete(MakeReal(path))) break;
        boost::this_thread::sleep(boost::posix_time::microseconds(10000));
        continue;
      }
      
      data.State().Update(len);
      
      char *bufp = buffer;
      if (data.DataType() == ftp::DataType::ASCII)
      {
        ftp::ASCIITranscodeRETR(buffer, len, asciiBuf);
        len = asciiBuf.size();
        bufp = asciiBuf.data();
      }
      
      data.Write(bufp, len);

      if (client.Profile().MaxDlSpeed() > 0)
        ftp::SpeedLimitSleep(data.State(), client.Profile().MaxDlSpeed());
    }
  }
  catch (const std::ios_base::failure& e)
  {
    fin->close();
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                 "Error while reading from disk: " + std::string(e.what()));
    throw cmd::NoPostScriptError();
  }
  catch (const util::net::NetworkError& e)
  {
    fin->close();
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                 "Error while writing to data connection: " +
                 e.Message());
    throw cmd::NoPostScriptError();
  }
  
  fin->close();
  data.Close();
  pt::time_duration duration = data.State().EndTime() - data.State().StartTime();
  db::stats::Download(client.User(), bytes / 1024, duration.total_milliseconds());
  acl::UserCache::DecrCredits(client.User().Name(), bytes / 1024);

  control.Reply(ftp::DataClosedOkay, "Transfer finished @ " + 
      stats::util::AutoUnitSpeedString(stats::util::CalculateSpeed(data.State().Bytes(), duration))); 
  
  (void) countGuard;
}

} /* rfc namespace */
} /* cmd namespace */
