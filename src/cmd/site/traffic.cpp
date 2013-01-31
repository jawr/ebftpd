#include <sstream>
#include <vector>
#include <map>
#include "cmd/site/traffic.hpp"
#include "db/stats/protocol.hpp"
#include "db/stats/transfers.hpp"
#include "stats/types.hpp"
#include "stats/util.hpp"
#include "util/string.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

std::string TRAFFICCommand::Format(const std::string& timeframe, 
    long long sendKBytes, long long receiveKBytes, const std::string& section)
{
  using namespace stats;

  std::ostringstream os;
  os << "| " << std::setw(9) << std::left << timeframe 
     << " | " << std::setw(9) << section.substr(0, 9) 
     << " | " << std::setw(10) << std::right
     << AutoUnitString(receiveKBytes).substr(0,10) 
     << "| " << std::setw(10) 
     << AutoUnitString(sendKBytes).substr(0,10) << " " 
     << " |";
  return os.str();
}

void TRAFFICCommand::Execute()
{
  std::map<stats::Timeframe, std::pair<long long, long long>> combined;
    
  std::ostringstream os;
  os << ".-------------------------------------------------.\n"
     << "| Transfer traffic:                               |\n"
     << "+-----------.-----------.------------.------------+\n"
     << "| Timeframe | Section   |   Incoming |   Outgoing |\n"
     << "+-----------.-----------+------------+------------+\n";
     
  std::vector<std::string> sections;
  for (auto& kv : cfg::Get().Sections()) sections.push_back(kv.first);
  sections.push_back("");

  for (auto& section : sections)
  {
    for (auto tf : ::stats::timeframes)
    {
      db::stats::Traffic t(db::stats::TransfersTotal(tf, section));
      os << Format(util::string::TitleSimpleCopy(
              ::util::EnumToString(tf)),
              t.SendKBytes(), t.ReceiveKBytes(), section) << "\n";
      combined[tf].first += t.SendKBytes();
      combined[tf].second += t.ReceiveKBytes();    
    }
  }
     
  os << "|-----------'-----------'------------'------------+\n"
     << "| Protocol traffic:                               |\n"
     << "|-----------.-----------.------------.------------|\n";

  for (auto tf : ::stats::timeframes)
  {
    db::stats::Traffic t(db::stats::ProtocolTotal(tf));
    os << Format(util::string::TitleSimpleCopy(
            ::util::EnumToString(tf)), 
            t.SendKBytes(), t.ReceiveKBytes()) << "\n";
    combined[tf].first += t.SendKBytes();
    combined[tf].second += t.ReceiveKBytes();
  }
  
  os << "|-----------'-----------'------------'------------+\n"
     << "| Combined traffic:                               |\n"
     << "|-----------.-----------.------------.------------|\n";

   for (auto tf : ::stats::timeframes)
  {
    os << Format(util::string::TitleSimpleCopy(
            ::util::EnumToString(tf)), combined[tf].first, 
        combined[tf].second) << "\n";
  }
     
  os << "`-----------'-----------'------------'------------'\n";
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
