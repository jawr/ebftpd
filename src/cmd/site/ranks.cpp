#include "cmd/site/ranks.hpp"
#include "util/string.hpp"
#include "stats/types.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "acl/acl.hpp"
#include "acl/misc.hpp"
#include "acl/user.hpp"
#include "stats/compile.hpp"

namespace cmd { namespace site
{

void RANKSCommand::Execute()
{
  if (args[0] == "RANKS")
  {
    if (!acl::AllowSiteCmd(client.User(), "ranks")) throw cmd::PermissionError();
  }
  else
  {
    if (!acl::AllowSiteCmd(client.User(), "ranksalias")) throw cmd::PermissionError();
  }

  ::stats::Timeframe tf;
  if (!util::EnumFromString(args[1], tf)) throw cmd::SyntaxError();

  ::stats::Direction dir;
  if (!util::EnumFromString(args[2], dir)) throw cmd::SyntaxError();
  
  ::stats::SortField sf;
  if (!util::EnumFromString(args[3], sf)) throw cmd::SyntaxError();

  int number = 10;
  if (args.size() >= 5)
  {
    try
    {
      number = boost::lexical_cast<int>(args[4]);
      if (number < 0) throw boost::bad_lexical_cast();
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw cmd::SyntaxError();
    }
  }

  int maxNumber = acl::stats::MaxUsers(client.User());
  std::cout << maxNumber << std::endl;
  if (maxNumber != -1)
  {
    number = std::min(maxNumber, number);
  }
  
  const cfg::Config& config = cfg::Get();

  std::string section;
  if (args.size() >= 6)
  {
    section = util::ToUpperCopy(args[5]);
    if (config.Sections().find(section) == config.Sections().end())
    {
      control.Reply(ftp::ActionNotOkay, "Section " + section + " doesn't exist.");
      return;
    }
  }
  
  acl::ACL acl("*");
  if (args.size() >= 7)
  {
    std::vector<std::string> aclArgs(args.begin() + 6, args.end());
    acl = acl::ACL(util::Join(aclArgs, " "));
  }
  
  std::string tmplName = "ranks." + util::EnumToString(tf) + 
                         "." + util::EnumToString(dir) + 
                         "." + util::EnumToString(sf);
                         
  boost::optional<text::Template> templ;
  try
  {
    try
    {
      templ.reset(text::Factory::GetTemplate(tmplName));
    }
    catch (const text::TemplateError&)
    {
      templ.reset(text::Factory::GetTemplate("ranks"));
    }
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::string message = stats::CompileUserRanks(section, tf, dir, sf, number, *templ, 
                          [&](const acl::User& user)
                          {
                            return acl.Evaluate(user.ACLInfo());
                          });
  
  control.Reply(ftp::CommandOkay, message);
}

} /* site namespace */
} /* cmd namespace */
