#include <sstream>
#include "cmd/site/delip.hpp"

namespace cmd { namespace site
{

void DELIPCommand::Execute()
{
  static const char* syntax = "Syntax: SITE DELIP <user> [ident@ip#1> ... <ident@ip#n>]";

  if (args.size() < 3)
  {
    control.Reply(ftp::SyntaxError, syntax);
    return;
  }

  std::ostringstream os;
  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    os << "Error: " << e.Message();
    control.Reply(ftp::ActionNotOkay, os.str());
    return;
  }

  util::Error ipDeleted;
  for (Args::iterator it = args.begin()+2; it != args.end(); ++it)
  {
    if (it != args.begin()+2) os << "\n";
    ipDeleted = acl::IpMaskCache::Delete(user, (*it));
    if (ipDeleted)
      os << "IP (" << (*it) << ") removed from " << args[1];
    else
      os << "Error: " << ipDeleted.Message();
  }

  control.MultiReply(ftp::CommandOkay, os.str());
}

// end
}
}
