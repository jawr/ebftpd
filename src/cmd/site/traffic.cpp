#include <sstream>
#include <map>
#include "cmd/site/traffic.hpp"
#include "db/stats/protocol.hpp"
#include "stats/types.hpp"
#include "stats/util.hpp"
#include "util/string.hpp"

namespace cmd { namespace site
{

std::string TRAFFICCommand::Format(const std::string& timeframe, 
    long long sendBytes, long long receiveBytes, const std::string& section)
{
  using namespace stats::util;

  std::ostringstream os;
  os << "| " << std::setw(9) << std::left << timeframe 
     << " | " << std::setw(9) << section.substr(0, 9) 
     << " | " << std::setw(10) << std::right
     << AutoUnitString(sendBytes).substr(0,10) << " " 
     << "| " << std::setw(10) 
     << AutoUnitString(receiveBytes).substr(0,10) 
     << " |";
  return os.str();
}

void TRAFFICCommand::Execute()
{
  static stats::Timeframe timeframes[] =
  { stats::Timeframe::Day, stats::Timeframe::Week, 
    stats::Timeframe::Month, stats::Timeframe::Year, 
    stats::Timeframe::Alltime };

  std::map<stats::Timeframe, std::pair<long long, long long>> combined;
    
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
     << "|-----------.-----------.------------.------------|\n";

  for (auto tf : timeframes)
  {
    db::stats::Protocol t(db::stats::CalculateTotal(tf));
    os << Format(util::string::TitleCaseCopy(
            ::util::EnumToString(tf)), 
            t.SendBytes(), t.ReceiveBytes()) << "\n";
    combined[tf].first += t.SendBytes();
    combined[tf].second += t.ReceiveBytes();
  }
  
  os << "|-----------'-----------'------------'------------+\n"
     << "| Combined traffic:                               |\n"
     << "|-----------.-----------.------------.------------|\n";

   for (auto tf : timeframes)
  {
    os << Format(util::string::TitleCaseCopy(
            ::util::EnumToString(tf)), combined[tf].first, 
        combined[tf].second) << "\n";
  }
     
  os << "`-----------'-----------'------------'------------'\n";
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
