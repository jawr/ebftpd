#include <ctype.h>
#include <ostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "cmd/site/give.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "acl/userprofile.hpp"
#include "util/error.hpp"
#include "logs/logs.hpp"
#include "acl/allowsitecmd.hpp"
#include "db/user/userprofile.hpp"
#include "cfg/get.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void GIVECommand::Execute()
{
  std::string section;
  if (boost::to_lower_copy(args[1]) == "-s")
  {
    section = boost::to_upper_copy(args[2]);
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

  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::string amount = args[2];
  std::string type = "K";
  if (isalpha(amount.at(amount.length()-1)))
  {
    type.assign(amount.end()-1, amount.end());
    amount.assign(amount.begin(), amount.end()-1);
    boost::to_upper(type);
  }

  long long credits;
  try
  {
    credits = boost::lexical_cast<long long>(amount);
    if (credits < 0) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast& e)
  {
    throw cmd::SyntaxError();
  }

  if (type == "G")
    credits *= 1024 * 1024 * 1024;
  else if (type == "M")
    credits *= 1024 * 1024;
  else
    credits *= 1024;

  std::ostringstream os;
  if (acl::AllowSiteCmd(client.User(), "giveown") &&
      !acl::AllowSiteCmd(client.User(), "give"))
  {
    try
    {
      acl::UserProfile profile = db::userprofile::Get(user.UID());
      int ratio = profile.Ratio(section);
      if (ratio == 0 || (ratio == -1 && profile.Ratio("") == 0))
      {
        control.Reply(ftp::ActionNotOkay, "Not allowed to give credits when you have leech!");
        return;
      }
    }
    catch (const util::RuntimeError& e)
    {
      control.Reply(ftp::ActionNotOkay, "Unable to load your user profile.");
      return;
    }
    // take away users credits/warn them
    
    if (!db::userprofile::DecrCredits(client.User().UID(), credits, section, false))
    {
      os << "Not enough credits to do that.";
      control.Reply(ftp::ActionNotOkay, os.str());
      return;
    }
    
    os << "Taken " << credits << "KB credits from you!\n";
  }
  
  // give user the credits
  db::userprofile::IncrCredits(user.UID(), credits, section);
  os << "Given " << std::setprecision(2) << std::fixed << credits / 1024.0 
     << "KB credits to " << user.Name() << ".";
  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
