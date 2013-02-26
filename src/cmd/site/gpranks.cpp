#include "cmd/site/gpranks.hpp"
#include "stats/types.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "acl/misc.hpp"
#include "stats/compile.hpp"
#include "util/string.hpp"

namespace cmd { namespace site

{

void GPRANKSCommand::Execute()
{
  if (args[0] == "RANKS")
  {
    if (!acl::AllowSiteCmd(client.User(), "gpranks")) throw cmd::PermissionError();
  }
  else
  {
    if (!acl::AllowSiteCmd(client.User(), "gpranksalias")) throw cmd::PermissionError();
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
  
  int maxNumber = acl::stats::MaxGroups(client.User());
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
  
  std::string tmplName = "gpranks." + util::EnumToString(tf) + 
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
      templ.reset(text::Factory::GetTemplate("gpranks"));
    }
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::string message = stats::CompileGroupRanks(section, tf, dir, sf, number, *templ);
  control.Reply(ftp::CommandOkay, message);
}

} /* site namespace */
} /* cmd namespace */
