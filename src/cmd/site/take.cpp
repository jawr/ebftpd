#include <ctype.h>
#include <ostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "cmd/site/take.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "acl/userprofile.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

cmd::Result TAKECommand::Execute()
{
  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, "Error: " + e.Message());
    return cmd::Result::Okay;
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
    return cmd::Result::Okay;
  }

  if (type == "G")
    credits *= 1000 * 1000;
  else if (type == "M")
    credits *= 1000;

  acl::UserCache::DecrCredits(user.Name(), credits);
  control.Reply(ftp::CommandOkay, "Taken " + args[2] + " from " + args[1] + ".");
  return cmd::Result::Okay;
}

// end
}
}

