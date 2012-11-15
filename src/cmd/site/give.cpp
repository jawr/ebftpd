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

namespace cmd { namespace site
{

void GIVECommand::Execute()
{
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
  long credits;
  if (isalpha(amount.at(amount.length()-1)))
  {
    type.assign(amount.end()-1, amount.end());
    amount.assign(amount.begin(), amount.end()-1);
    boost::to_upper(type);
  }

  try
  {
    credits = boost::lexical_cast<long>(amount);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    control.Reply(ftp::ActionNotOkay, "Error parsing number!");
    return;
  }

  if (type == "G")
    credits *= 1024 * 1024;
  else if (type == "M")
    credits *= 1024;

  std::ostringstream os;
  if (acl::AllowSiteCmd(client.User(), "giveown") &&
      !acl::AllowSiteCmd(client.User(), "give"))
  {
    try
    {
      acl::UserProfile profile = db::userprofile::Get(user.UID());
      if (profile.Ratio() == 0)
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
    
    if (client.User().Credits() < credits)
    {
      os << "You only have " << client.User().Credits() << "KB credits left.";
      control.Reply(ftp::ActionNotOkay, os.str());
      return;
    }
    
    acl::UserCache::DecrCredits(client.User().Name(), credits);
    os << "Taken " << credits << "KB credits from you!\n";
  }
  // give user the credits
  acl::UserCache::IncrCredits(user.Name(), credits);
  os << "Given " << credits << "KB credits to " << user.Name() << ".";
  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
