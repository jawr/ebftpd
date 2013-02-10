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
#include "text/error.hpp"
#include "text/factory.hpp"
#include "db/stats/traffic.hpp"

namespace cmd { namespace site
{

void TRAFFICCommand::Execute()
{
  std::map<stats::Timeframe, std::pair<long long, long long>> combined;
    
  boost::optional<text::Template> tmpl;
  try
  {
    tmpl.reset(text::Factory::GetTemplate("traffic"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
     
  std::vector<std::string> sections;
  for (auto& kv : cfg::Get().Sections())
  {
    sections.emplace_back(kv.first);
  }
  
  sections.emplace_back("");

  auto& body = tmpl->Body();
  
  std::ostringstream tos;
  for (auto& section : sections)
  {
    body.RegisterValue("section", section);
    for (auto tf : ::stats::timeframes)
    {
      db::stats::Traffic t(db::stats::TransfersTotal(tf, section));

      std::string prefix = util::EnumToString(tf) + "_";
      body.RegisterSize(prefix + "send", t.SendKBytes());
      body.RegisterSize(prefix + "receive", t.ReceiveKBytes());
      combined[tf].first += t.SendKBytes();
      combined[tf].second += t.ReceiveKBytes();    
    }
    tos << body.Compile();
  }
     
  auto& head = tmpl->Head();
  auto& foot = tmpl->Foot();

  for (auto tf : ::stats::timeframes)
  {
    db::stats::Traffic t(db::stats::ProtocolTotal(tf));

    std::string prefix = "protocol_" + util::EnumToString(tf) + "_";
    head.RegisterSize(prefix + "send", t.SendKBytes());
    head.RegisterSize(prefix + "receive", t.ReceiveKBytes());
    foot.RegisterSize(prefix + "send", t.SendKBytes());
    foot.RegisterSize(prefix + "receive", t.ReceiveKBytes());

    combined[tf].first += t.SendKBytes();
    combined[tf].second += t.ReceiveKBytes();
  }
  
  for (auto tf : ::stats::timeframes)
  {
    std::string prefix = "total_" + util::EnumToString(tf) + "_";
    head.RegisterSize(prefix + "send", combined[tf].first);
    head.RegisterSize(prefix + "receive", combined[tf].second);
    foot.RegisterSize(prefix + "send", combined[tf].first);
    foot.RegisterSize(prefix + "receive", combined[tf].second);
  }
  
  std::ostringstream os;
  os << head.Compile();
  os << tos.str();
  os << foot.Compile();
     
  //os << "`-----------'-----------'------------'------------'\n";
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
