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
    os << std::setprecision(2) << (double)down.Kbytes()/1000.0 << " MiB ";
  }
  catch (const util::RuntimeError& e)
  {
  }

  try
  {
    ::stats::Stat up = db::stats::GetWeekUp(client.User().UID(), 43, 2012);
    os << "Up - Files: " << up.Files() << " ";
    os << std::setprecision(2) << (double)up.Kbytes()/1000.0 << " MiB";
  }
  catch (const util::RuntimeError& e)
  {
  }

  if (os.str().empty())
    control.Reply(ftp::ActionNotOkay, "Error: No stats for user.");
  else
    control.Reply(ftp::CommandOkay, os.str());

  return cmd::Result::Okay;
}

// end
}
}
