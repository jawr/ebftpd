#include <sstream>
#include "cmd/site/addip.hpp"
#include "util/error.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void ADDIPCommand::Execute()
{
  static const char* syntax = "Syntax: SITE ADDIP <user> [<ident@ip#1> ... <ident@ip#n>]";

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

  std::vector<std::string> deleted;
  util::Error ipOkay;
  for (Args::iterator it = args.begin()+2; it != args.end(); ++it)
  {
    deleted.clear();
    ipOkay = acl::IpMaskCache::Add(user, (*it), deleted);

    if (!ipOkay)
    {
      os << "\n\tError adding " << (*it) << ": " << ipOkay.Message();
      continue;
    }

    os << "\nIP '" << (*it) << "' successfully added to " << args[1] << ".";

    if (deleted.size() > 0)
      for (auto& del: deleted)
        os << "\nAuto-removing useless IP '" << del << "'...";
  }

  control.MultiReply(ftp::CommandOkay, os.str());
} 

// end 
}
}
