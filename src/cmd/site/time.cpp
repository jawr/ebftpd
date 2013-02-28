#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/site/time.hpp"

namespace cmd { namespace site
{

namespace pt = boost::posix_time;

namespace
{

std::string FormatDuration(const boost::posix_time::time_duration& duration)
{
  std::ostringstream os;
  if (duration.hours()) os << duration.hours() << "h ";
  if (duration.minutes()) os << duration.minutes() << "m ";
  if (duration.seconds()) os << duration.seconds() << "s";
  return os.str();
}

}

void TIMECommand::Execute()
{
  pt::ptime now = pt::second_clock::local_time();
  
  std::ostringstream os;
  os << "Current time : " << now << "\n"
     << "Logged in at : " << client.LoggedInAt() << "\n"
     << "Time online  : " << FormatDuration(now - client.LoggedInAt());
  
  control.Reply(ftp::CommandOkay, os.str());
  return;
}

} /* site namespace */
} /* cmd namespace */
