#include <ctype.h>
#include <ostream>
#include <boost/lexical_cast.hpp>
#include "cmd/site/take.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"
#include "cmd/util.hpp"

namespace cmd { namespace site
{

void TAKECommand::Execute()
{
  std::string section;
  if (util::ToLowerCopy(args[1]) == "-s")
  {
    section = util::ToUpperCopy(args[2]);
    if (args.size() < 5) throw cmd::SyntaxError();
    args.erase(args.begin() + 1, args.begin() + 3);
    
    const cfg::Config& config = cfg::Get();
    auto it = config.Sections().find(section);
    if (it == config.Sections().end())
    {
      control.Reply(ftp::ActionNotOkay, "Section " + section + " doesn't exist.");
      return;
    }
    
    if (!it->second.SeparateCredits())
    {
      control.Reply(ftp::ActionNotOkay, "Section " + section + " dosen't have separate credits.");
      return;
    }
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }

  long long credits;
  if (!ParseCredits(args[2], credits)) throw cmd::SyntaxError();

  user->DecrSectionCreditsForce(section, credits);
  
  std::ostringstream os;
  os << "Taken " << std::fixed << std::setprecision(2) << credits
     << "KB credits from " << user->Name();
  if (!section.empty()) os << " on section " << section;
  os << ".";
  control.Reply(ftp::CommandOkay, os.str());
  
  if (section.empty())
    logs::Siteop(client.User().Name(), "took '%2%' credits from '%1%'", user->Name(), credits);
  else
    logs::Siteop(client.User().Name(), "took '%2%' credits from '%1%' on section '%3%'", user->Name(), credits, section);
}

// end
}
}

