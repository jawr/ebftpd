#include <ios>
#include <unistd.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/crc.hpp>
#include "cmd/rfc/stor.hpp"
#include "fs/file.hpp"
#include "db/stats/stat.hpp"
#include "acl/usercache.hpp"
#include "stats/util.hpp"
#include "ftp/counter.hpp"
#include "util/scopeguard.hpp"
#include "ftp/util.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "acl/path.hpp"
#include "fs/chmod.hpp"
#include "fs/mode.hpp"
#include "util/string.hpp"
#include "exec/check.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace rfc
{

namespace 
{
const fs::Mode completeMode(fs::Mode("0666"));

std::string CRCToString(boost::crc_32_type& crc)
{
  std::ostringstream os;
  os << std::hex << std::uppercase << crc.checksum();
  return os.str();
}

}

void STORCommand::DupeMessage(const fs::VirtualPath& path)
{
  std::ostringstream os;
  os << ftp::xdupe::Message(client, path);
  os << "Unable to create file: " << util::SystemError(EEXIST).Message();
  control.Reply(ftp::BadFilename, os.str());
}

bool STORCommand::CalcCRC(const fs::VirtualPath& path)
{
  for (auto& mask : cfg::Get().CalcCrc())
  {
    if (util::string::WildcardMatch(mask, path.ToString())) return true;
  }
  
  return false;
}

void STORCommand::Execute()
{
  namespace pt = boost::posix_time;

  using util::scope_guard;
  using util::make_guard;

  fs::VirtualPath path(fs::PathFromUser(argStr));
  
  fs::Path messagePath;
  util::Error e(acl::path::Filter(client.User(), path.Basename(), messagePath));
  if (!e)
  {
    // should display above messagepath, we'll just reply for now
    control.Reply(ftp::ActionNotOkay, "File name contains one or more invalid characters.");
    throw cmd::NoPostScriptError();
  }

  off_t offset = data.RestartOffset();
  if (offset > 0 && data.DataType() == ftp::DataType::ASCII)
  {
    control.Reply(ftp::BadCommandSequence, "Resume not supported on ASCII data type.");
    throw cmd::NoPostScriptError();
  }
  
  if (!exec::PreCheck(client, path)) throw cmd::NoPostScriptError();

  if (!ftp::Counter::StartUpload(client.User().UID(), client.Profile().MaxSimUl()))
  {
    std::ostringstream os;
    os << "You have reached your maximum of " << client.Profile().MaxSimUl() 
       << " simultaenous upload(s).";
    control.Reply(ftp::ActionNotOkay, os.str());
    throw cmd::NoPostScriptError();
  }
  
  scope_guard countGuard = make_guard([&]{ ftp::Counter::StopUpload(client.User().UID()); });  

  if (data.DataType() == ftp::DataType::ASCII &&
     !cfg::Get().AsciiUploads().Allowed(path.ToString()))
  {
    control.Reply(ftp::ActionNotOkay, "File can't be uploaded in ASCII, change to BINARY.");
    throw cmd::NoPostScriptError();
  }
  
  fs::FileSinkPtr fout;
  try
  {
    if (data.RestartOffset() > 0)
      fout = fs::AppendFile(client, path, data.RestartOffset());
    else
      fout = fs::CreateFile(client, path);
  }
  catch (const util::SystemError& e)
  {
    if (data.RestartOffset() == 0 && e.Errno() == EEXIST)
    {
      DupeMessage(path);
    }
    else
    {
      std::ostringstream os;
      os << "Unable to " << (data.RestartOffset() > 0 ? "append" : "create")
         << " file: " << e.Message();
      control.Reply(ftp::ActionNotOkay, os.str());
    }
    throw cmd::NoPostScriptError();
  }

  std::stringstream os;
  os << "Opening " << (data.DataType() == ftp::DataType::ASCII ? "ASCII" : "BINARY") 
     << " connection for upload of " 
     << fs::MakePretty(path).ToString();
  if (data.Protection()) os << " using TLS/SSL";
  os << ".";
  control.Reply(ftp::TransferStatusOkay, os.str());
  
  try
  {
    data.Open(ftp::TransferType::Upload);
  }
  catch (const util::net::NetworkError&e )
  {
    if (!data.RestartOffset()) fs::DeleteFile(fs::MakeReal(path));
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    throw cmd::NoPostScriptError();
  }
  
  bool calcCrc = CalcCRC(path);
  boost::crc_32_type crc;
  
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
        ftp::ASCIITranscodeSTOR(buffer, len, asciiBuf);
        len = asciiBuf.size();
        bufp = asciiBuf.data();
      }
      
      data.State().Update(len);
      
      fout->write(bufp, len);
      
      if (calcCrc) crc.process_bytes(bufp, len);
      
      if (client.Profile().MaxUlSpeed() > 0)
        ftp::SpeedLimitSleep(data.State(), client.Profile().MaxUlSpeed());
    }
  }
  catch (const util::net::EndOfStream&) { }
  catch (const util::net::NetworkError& e)
  {
    fout->close();
    data.Close();
    fs::DeleteFile(fs::MakeReal(path));
    control.Reply(ftp::DataCloseAborted,
                 "Error while reading from data connection: " +
                 e.Message());
    throw cmd::NoPostScriptError();
  }
  catch (const std::ios_base::failure& e)
  {
    fout->close();
    data.Close();
    fs::DeleteFile(fs::MakeReal(path));
    control.Reply(ftp::DataCloseAborted,
                  "Error while writing to disk: " + std::string(e.what()));
  }
  
  fout->close();
  data.Close();
  
  e = fs::Chmod(client, path, completeMode);
  if (!e) control.PartReply(ftp::DataClosedOkay, "Failed to chmod upload: " + e.Message());

  boost::posix_time::time_duration duration = data.State().EndTime() - data.State().StartTime();
  double speed = stats::util::CalculateSpeed(data.State().Bytes(), duration);
  
  if (!exec::PostCheck(client, path, calcCrc ? CRCToString(crc) : "000000", speed, "UNKNOWN"))
    fs::DeleteFile(fs::MakeReal(path));
  else
  {
    db::stats::Upload(client.User(), data.State().Bytes(), duration.total_milliseconds());
    long long credits = data.State().Bytes() * client.UserProfile().Ratio();
    acl::UserCache::IncrCredits(client.User().Name(), credits);
  }

  control.Reply(ftp::DataClosedOkay, "Transfer finished @ " + 
      stats::util::AutoUnitSpeedString(speed)); 
  
  (void) countGuard;
}

} /* rfc namespace */
} /* cmd namespace */
