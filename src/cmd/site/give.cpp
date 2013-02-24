#include <ctype.h>
#include <ostream>
#include <boost/lexical_cast.hpp>
#include "cmd/site/give.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "util/error.hpp"
#include "acl/misc.hpp"
#include "cfg/get.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"
#include "cmd/util.hpp"

namespace cmd { namespace site
{

void GIVECommand::Execute()
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

  bool own = false;
  std::ostringstream os;
  if (acl::AllowSiteCmd(client.User(), "giveown") &&
      !acl::AllowSiteCmd(client.User(), "give"))
  {
    int ratio = client.User().SectionRatio(section);
    if (ratio == 0 || (ratio == -1 && client.User().DefaultRatio() == 0))
    {
      control.Reply(ftp::ActionNotOkay, "Not allowed to give credits when you have leech!");
      return;
    }

    // take away users credits/warn them
    
    if (!client.User().DecrSectionCredits(section, credits))
    {
      control.Reply(ftp::ActionNotOkay, "Not enough credits to do that.");
      return;
    }
    
    os << "Taken " << credits << "KB credits from you!\n";
    own = true;
  }
  
  // give user the credits
  user->IncrSectionCredits(section, credits);
  os << "Given " << std::setprecision(2) << std::fixed << credits
     << "KB credits to " << user->Name();
  if (!section.empty()) os << " on section " << section;
  os << ".";
  control.Reply(ftp::CommandOkay, os.str());
  
  if (!own)
  {
    if (section.empty())
      logs::Siteop(client.User().Name(), "gave '%2%' credits to '%1%'", user->Name(), credits);
    else
      logs::Siteop(client.User().Name(), "gave '%2%' credits to '%1%' on section '%3%'", 
                   user->Name(), credits, section);
  }
}

// end
}
}
