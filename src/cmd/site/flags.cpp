#include <sstream>
#include "cmd/site/flags.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "acl/flags.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

void FLAGSCommand::Execute()
{
  static const char* syntax = "Syntax: SITE FLAGS <user>";

  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, syntax);
    return;
  }

  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, "Error: " + e.Message());
    return;
  }

  std::ostringstream os;
  os << "FLAGS for " << args[1] << "\n\n";
  os << CheckFlag(user, acl::Flag::FlagSiteop) << " Siteop -1-\n";
  os << CheckFlag(user, acl::Flag::Gadmin) << " Gadmin -2-\n";
  os << CheckFlag(user, acl::Flag::Glock) << " Glock -3-\n";
  os << CheckFlag(user, acl::Flag::Exempt) << " Exempt -4-\n";
  os << CheckFlag(user, acl::Flag::Color) << " Color -5-\n";
  os << CheckFlag(user, acl::Flag::Deleted) << " Deleted -6-\n";
  os << CheckFlag(user, acl::Flag::Useredit) << " User Edit -7-\n";
  os << CheckFlag(user, acl::Flag::Anonymous) << " Anonymous -8-\n";
  os << CheckFlag(user, acl::Flag::Nuke) << " Nuke -A-\n";
  os << CheckFlag(user, acl::Flag::Unnuke) << " Unnuke -B-\n";
  os << CheckFlag(user, acl::Flag::Undupe) << " Undupe -C-\n";
  os << CheckFlag(user, acl::Flag::Kick) << " Kick -D-\n";
  os << CheckFlag(user, acl::Flag::Kill) << " Kill -E-\n";
  os << CheckFlag(user, acl::Flag::Take) << " Take -F-\n";
  os << CheckFlag(user, acl::Flag::Give) << " Give -G-\n";
  os << CheckFlag(user, acl::Flag::Users) << " Users -H-\n";
  os << CheckFlag(user, acl::Flag::Idler) << " Idler -I-\n";
  os << CheckFlag(user, acl::Flag::Custom1) << " Custom 1 -J-\n";
  os << CheckFlag(user, acl::Flag::Custom2) << " Custom 2 -K-\n";
  os << CheckFlag(user, acl::Flag::Custom3) << " Custom 3 -L-\n";
  os << CheckFlag(user, acl::Flag::Custom4) << " Custom 4 -M-\n";
  os << CheckFlag(user, acl::Flag::Custom5) << " Custom 5 -N-\n\n";

  os << "A '*' denotes an enabled flag.  Only sysop users may change flags.\n";
  os << "Use SITE CHANGE <USER> FLAGS +/-VALUE to change.";

  control.MultiReply(ftp::CommandOkay, os.str());

}

// end
}
}
