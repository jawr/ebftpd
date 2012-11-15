#include <sstream>
#include <map>
#include "cmd/site/group.hpp"
#include "util/error.hpp"
#include "acl/groupcache.hpp"
#include "acl/group.hpp"
#include "acl/user.hpp"
#include "acl/flags.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "util/time.hpp"
#include "stats/stat.hpp"
#include "db/stats/stat.hpp"
#include "stats/conversions.hpp"

namespace cmd { namespace site
{

void GROUPCommand::Execute()
{
  acl::Group group;
  try
  {
    group = acl::GroupCache::Group(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::vector<acl::User> users = db::user::GetByACL("=" + args[1]);

  if (users.empty())
  {
    control.Reply(ftp::ActionNotOkay, "SITE GROUP: No users.");
    return;
}

  std::map<acl::UserID, acl::UserProfile> profiles = 
    db::userprofile::GetSelection(users);

  std::map<acl::UserID, ::stats::Stat> dnStats = db::stats::GetAllDown(users);
  std::map<acl::UserID, ::stats::Stat> upStats = db::stats::GetAllUp(users);

  std::ostringstream os;
  os << ",-----------+--------+-----------+--------+-----------+-------+---------.";
  os << "\n|  Username |     Up |      Megs |     Dn |      Megs | Ratio |    Wkly |";
  os << "\n|-----------+--------+-----------+--------+-----------+-------+---------|";

  for (auto& user: users)
  {
    os << "\n| ";
    std::string flag = (user.CheckFlag(acl::Flag::Gadmin)) ? "+" : " ";
    flag = (user.CheckFlag(acl::Flag::Siteop)) ? "*" : flag;
    os << flag << std::left << std::setw(8) << user.Name().substr(0, 8) << " | ";
    os << std::right << std::setw(6) << upStats[user.UID()].Files() << " | ";
    os << std::right << std::setw(9) 
      << ::stats::tostring::Mbyte(upStats[user.UID()]) << " | ";
    os << std::right << std::setw(6) << dnStats[user.UID()].Files() << " | ";
    os << std::right << std::setw(9) 
      << ::stats::tostring::Mbyte(dnStats[user.UID()]) << " | ";
    os << std::right << std::setw(5) << profiles[user.UID()].Ratio() << " | "; 
    os << std::right << std::setw(7) << profiles[user.UID()].WeeklyAllotment() << " | ";
  }

  os << "\n|-----------+--------+-----------+--------+-----------+-------+---------|";
  os << "\n|   * denotes a siteop (flag 1)     + denotes a group admin (flag 2)    |";
  os << "\n|----------+------+--------+------+--------+-------+--------------------|";
  os << "\n|   Free Ratio Slots: Unlimited   Free Leech Slots: 0                   |";
  os << "\n`-----------------------------------------------------------------------'";

  control.Reply(ftp::CommandOkay, os.str()); 
}

// end
}
}
