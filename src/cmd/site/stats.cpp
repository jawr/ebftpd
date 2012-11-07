#include <ostream>
#include "cmd/site/stats.hpp"
#include "stats/stat.hpp"
#include "db/stats/stat.hpp"

namespace cmd { namespace site
{

cmd::Result STATSCommand::Execute()
{
  std::ostringstream os;
  os.imbue(std::locale(""));
  try
  {
    ::stats::Stat down = db::stats::GetWeekDown(client.User().UID(), 43, 2012);
    os << "Down - Files: " << down.Files() << " ";
    os << std::setprecision(2) << down.Kbytes()/1024.0 << " MiB ";
  }
  catch (const util::RuntimeError& e)
  {
  }

  try
  {
    ::stats::Stat up = db::stats::GetWeekUp(client.User().UID(), 43, 2012);
    os << "Up - Files: " << up.Files() << " ";
    os << std::setprecision(2) << up.Kbytes()/1024.0 << " MiB";
  }
  catch (const util::RuntimeError& e)
  {
  }

  if (os.str().empty())
    control.Reply(ftp::ActionNotOkay, "No stats for user.");
  else
    control.Reply(ftp::CommandOkay, os.str());

  return cmd::Result::Okay;
}

// end
}
}
