#include <ios>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/rfc/retr.hpp"
#include "fs/file.hpp"
#include "db/stats/stats.hpp"
#include "stats/util.hpp"
#include "util/scopeguard.hpp"
#include "ftp/counter.hpp"
#include "ftp/util.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "cmd/error.hpp"
#include "acl/path.hpp"
#include "acl/misc.hpp"
#include "ftp/error.hpp"
#include "ftp/speedcontrol.hpp"
#include "acl/flags.hpp"
#include "ftp/data.hpp"
#include "fs/path.hpp"

namespace cmd { namespace rfc
{

void RETRCommand::Execute()
{
  namespace pt = boost::posix_time;
  
  off_t offset = data.RestartOffset();
  if (offset > 0 && data.DataType() == ftp::DataType::ASCII)
  {
    control.Reply(ftp::BadCommandSequence, "Resume not supported on ASCII data type.");
    throw cmd::NoPostScriptError();
  }
  
  switch(ftp::Counter::Download().Start(client.User().ID(), 
         client.User().MaxSimDown(), 
         client.User().HasFlag(acl::Flag::Exempt)))
  {
    case ftp::CounterResult::PersonalFail  :
    {
      std::ostringstream os;
      os << "You have reached your maximum of " << client.User().MaxSimDown() 
         << " simultaneous download(s).";
      control.Reply(ftp::ActionNotOkay, os.str());
      throw cmd::NoPostScriptError();
    }
    case ftp::CounterResult::GlobalFail    :
    {
      control.Reply(ftp::ActionNotOkay, 
          "The server has reached it's maximum number of simultaneous downloads.");
      throw cmd::NoPostScriptError();          
    }
    case ftp::CounterResult::Okay          :
      break;
  }
  
  auto countGuard = util::MakeScopeExit([&]{ ftp::Counter::Download().Stop(client.User().ID()); });  
  
  fs::VirtualPath path(fs::PathFromUser(argStr));

  fs::FileSourcePtr fin;
  try
  {
    fin = fs::OpenFile(client.User(), path);
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
  
  auto section = cfg::Get().SectionMatch(path.ToString());
  int ratio = stats::DownloadRatio(client, path, section);
  if (!client.User().DecrSectionCredits(section && section->SeparateCredits() ? 
          section->Name() : "", size / 1024 * ratio))
  {
    control.Reply(ftp::ActionNotOkay, "Not enough credits to download that file.");
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

  auto dataGuard = util::MakeScopeExit([&]
  {
    if (data.State().Type() != ftp::TransferType::None)
    {
      data.Close();
      db::stats::Download(client.User(), data.State().Bytes() / 1024, 
                          data.State().Duration().total_milliseconds());
    }
    
    if (size > data.State().Bytes())
    {
      // download failed short, give the remaining credits back
      client.User().IncrSectionCredits(section && section->SeparateCredits() ? 
              section->Name() : "", (size - data.State().Bytes()) / 1024 * ratio);
    }
    else
    if (data.State().Bytes() > size)
    {
      // final download size was larger than at start, take some more credits
      client.User().DecrSectionCreditsForce(section && section->SeparateCredits() ? 
              section->Name() : "", (data.State().Bytes() - size) * ratio);
    }
  });  

  if (!data.ProtectionOkay())
  {
    std::ostringstream os;
    os << "TLS is enforced on " << (data.IsFXP() ? "FXP" : "data") << " transfers.";
    control.Reply(ftp::ProtocolNotSupported, os.str());
    throw cmd::NoPostScriptError();
  }
  
  bool aborted = false;
  try
  {
    ftp::DownloadSpeedControl speedControl(client, path);
    bool dlIncomplete = cfg::Get().DlIncomplete();
    std::vector<char> asciiBuf;
    char buffer[16384];
    
    while (true)
    {
      std::streamsize len = fin->read(buffer, sizeof(buffer));
      if (len < 0) 
      {
        if (!dlIncomplete || !fs::IsIncomplete(MakeReal(path))) break;
        boost::this_thread::sleep(pt::microseconds(10000));
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
      speedControl.Apply();
    }
  }
  catch (const ftp::TransferAborted&) { aborted = true; }
  catch (const std::ios_base::failure& e)
  {
    control.Reply(ftp::DataCloseAborted,
                 "Error while reading from disk: " + std::string(e.what()));
    
    throw cmd::NoPostScriptError();
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::DataCloseAborted,
                 "Error while writing to data connection: " +
                 e.Message());

    throw cmd::NoPostScriptError();
  }
  catch (const ftp::ControlError& e)
  {
    e.Rethrow();
  }
  catch (const ftp::MinimumSpeedError& e)
  {
    logs::Debug("Aborted slow download by %1%. %2% lower than %3%", 
                client.User().Name(),
                stats::AutoUnitSpeedString(e.Speed()),
                stats::AutoUnitSpeedString(e.Limit()));
    aborted = true;
  }
  
  fin->close();
  data.Close();

  auto duration = data.State().Duration();
  bool nostats = !section || acl::path::FileAllowed<acl::path::Nostats>(client.User(), path);
  db::stats::Download(client.User(), data.State().Bytes() / 1024, duration.total_milliseconds(),
                      nostats ? "" : section->Name());
                      
  double speed = stats::CalculateSpeed(data.State().Bytes(), duration);
  
  if (aborted)
  {
    control.Reply(ftp::DataClosedOkay, "Transfer aborted @ " + stats::AutoUnitSpeedString(speed / 1024)); 
    throw cmd::NoPostScriptError();
  }

  control.Reply(ftp::DataClosedOkay, "Transfer finished @ " + stats::AutoUnitSpeedString(speed / 1024)); 
  
  (void) countGuard;
  (void) dataGuard;
}

} /* rfc namespace */
} /* cmd namespace */
