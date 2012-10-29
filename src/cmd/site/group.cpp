#include <sstream>
#include "cmd/site/group.hpp"
#include "util/error.hpp"
#include "acl/groupcache.hpp"
#include "acl/group.hpp"
#include "db/user/user.hpp"

namespace cmd { namespace site
{

cmd::Result GROUPCommand::Execute()
{
  acl::Group group;
  try
  {
    group = acl::GroupCache::Group(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, "Error: " + e.Message());
    return cmd::Result::Okay;
  }

  boost::ptr_vector<acl::User> users;
  util::Error ok = db::user::UsersByACL(users, "=" + args[1]);

  if (!ok)
  {
    control.Reply(ftp::ActionNotOkay, "Error: " + ok.Message());
    return cmd::Result::Okay;
  }

  std::ostringstream os;
  os << ",-----------+--------+-----------+--------+-----------+-------+---------.";
  os << "\n|  Username |     Up |      Megs |     Dn |      Megs | Ratio |    Wkly |";
  os << "\n|-----------+--------+-----------+--------+-----------+-------+---------|";

  for (auto& user: users)
  {
    os << "\n| " << user.Name();
  }

  os << "\n|-----------+--------+-----------+--------+-----------+-------+---------|";
  os << "\n|   * denotes a siteop (flag 1)     + denotes a group admin (flag 2)    |";
  os << "\n|----------+------+--------+------+--------+-------+--------------------|";
  os << "\n|   Free Ratio Slots: Unlimited   Free Leech Slots: 0                   |";
  os << "\n`-----------------------------------------------------------------------'";

  control.MultiReply(ftp::CommandOkay, os.str()); 
  return cmd::Result::Okay;
}

// end
}
}
