#include <sstream>
#include "cmd/site/traffic.hpp"
#include "db/stats/protocol.hpp"
#include "stats/types.hpp"
#include "stats/util.hpp"

namespace cmd { namespace site
{

std::string TRAFFICCommand::FormatProtocol(const std::string& timeframe, 
    const db::stats::ProtocolTotal& t)
{
  using namespace stats::util;

  std::ostringstream os;
  os << "| " << std::setw(9) << std::left << timeframe 
     << " |           | " << std::setw(10) << std::right
     << AutoUnitString(t.SendBytes()).substr(0,10) << " " 
     << "| " << std::setw(10) 
     << AutoUnitString(t.ReceiveBytes()).substr(0,10) 
     << " |";
  return os.str();
}

cmd::Result TRAFFICCommand::Execute()
{
  db::stats::ProtocolTotal day(db::stats::CalculateTotal(stats::Timeframe::Day));
  db::stats::ProtocolTotal week(db::stats::CalculateTotal(stats::Timeframe::Week));
  db::stats::ProtocolTotal month(db::stats::CalculateTotal(stats::Timeframe::Month));
  db::stats::ProtocolTotal year(db::stats::CalculateTotal(stats::Timeframe::Year));
  db::stats::ProtocolTotal alltime(db::stats::CalculateTotal(stats::Timeframe::Alltime));
  
  std::ostringstream os;

  os << ".-------------------------------------------------.\n"
     << "| Transfer traffic:                               |\n"
     << "+-----------.-----------.------------.------------+\n"
     << "| Timeframe | Section   |   Incoming |   Outgoing |\n"
     << "+-----------.-----------+------------+------------+\n"
     << "| Day       |           |            |            |\n"
     << "| Week      |           |            |            |\n"
     << "| Month     |           |            |            |\n"
     << "| Year      |           |            |            |\n"
     << "| Alltime   |           |            |            |\n"
     << "|-----------'-----------'------------'------------+\n"
     << "| Protocol traffic:                               |\n"
     << "|-----------.-----------.------------.------------|\n"
     << FormatProtocol("Day", day) << "\n"
     << FormatProtocol("Week", week) << "\n"
     << FormatProtocol("Month", month) << "\n"
     << FormatProtocol("Year", year) << "\n"
     << FormatProtocol("Alltime", alltime) << "\n"
     << "|-----------'-----------'------------'------------+\n"
     << "| Combined traffic:                               |\n"
     << "|-----------.-----------.------------.------------|\n"
     << "| Day       |           |            |            |\n"
     << "| Week      |           |            |            |\n"
     << "| Month     |           |            |            |\n"
     << "| Year      |           |            |            |\n"
     << "| Alltime   |           |            |            |\n"     
     << "`-----------'-----------'------------'------------'\n";
  
  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
