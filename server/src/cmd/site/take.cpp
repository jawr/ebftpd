#include <ctype.h>
#include <ostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/take.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "acl/userprofile.hpp"
#include "util/error.hpp"
#include "db/user/userprofile.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

void TAKECommand::Execute()
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
  long long credits;
  if (isalpha(amount.at(amount.length()-1)))
  {
    type.assign(amount.end()-1, amount.end());
    amount.assign(amount.begin(), amount.end()-1);
    boost::to_upper(type);
  }

  try
  {
    credits = boost::lexical_cast<long long>(amount);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    throw cmd::SyntaxError();
  }

  if (type == "G")
    credits *= 1024 * 1024 * 1024;
  else if (type == "M")
    credits *= 1024 * 1024;

  db::userprofile::DecrCredits(user.UID(), credits, section, true);
  
  std::ostringstream os;
  os << "Taken " << std::fixed << std::setprecision(2) << credits / 1024.0
     << "KB credits from " << user.Name() << ".";
  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}

