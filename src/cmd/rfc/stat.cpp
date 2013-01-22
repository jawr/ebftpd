#include "cmd/rfc/stat.hpp"
#include "cfg/get.hpp"
#include "main.hpp"
#include "stats/util.hpp"
#include "util/scopeguard.hpp"

namespace cmd { namespace rfc
{

void STATCommand::Execute()
{
  using util::scope_guard;
  using util::make_guard;

  if (args.size() == 1)
  {
    std::ostringstream os;
    os << programFullname << " status\n";
    os << "< Insert status info here >\n";
    os << "End of status.";
    control.Reply(ftp::SystemStatus, os.str());
    return;
  }
  
  bool singleLineReplies = control.SingleLineReplies();
  control.SetSingleLineReplies(false);
  
  scope_guard singleLineGuard = make_guard([&]{ control.SetSingleLineReplies(singleLineReplies); });  

  std::string options;
  std::string::size_type optOffset = 0;
  if (args[1][0] == '-')
  {
    options = args[1].substr(1);
    optOffset += args[1].length();
  }
  
  fs::Path path(boost::trim_copy(std::string(argStr, optOffset)));
  
  const cfg::Config& config = cfg::Get();
  std::string forcedOptions = "l" + config.Lslong().Options();
    
  control.PartReply(ftp::DirectoryStatus, "Status of " + fs::MakePretty(MakeVirtual(path)).ToString() + ":");
  DirectoryList dirList(client, control, path, ListOptions(options, forcedOptions),
                        config.Lslong().MaxRecursion());
  
  boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
  dirList.Execute();
  boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time();
  
  control.Reply(ftp::DirectoryStatus, "End of status (" + 
      stats::HighResSecondsString(start, end) + ")"); 
  return;
  
  (void) singleLineReplies;
  (void) singleLineGuard;
}

} /* rfc namespace */
} /* cmd namespace */
