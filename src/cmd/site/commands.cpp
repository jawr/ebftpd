//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <algorithm>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/posix_time_zone.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/regex.hpp>
#include <cassert>
#include <cctype>
#include <ctype.h>
#include <iomanip>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>
#include <ctime>
#include "cmd/site/commands.hpp"
#include "acl/acl.hpp"
#include "acl/flags.hpp"
#include "acl/group.hpp"
#include "acl/ipstrength.hpp"
#include "acl/misc.hpp"
#include "acl/passwdstrength.hpp"
#include "acl/path.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "acl/util.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "cfg/util.hpp"
#include "cmd/error.hpp"
#include "cmd/online.hpp"
#include "db/dupe/dupe.hpp"
#include "db/index/index.hpp"
#include "db/stats/protocol.hpp"
#include "db/stats/stats.hpp"
#include "db/stats/traffic.hpp"
#include "db/stats/transfers.hpp"
#include "fs/dircontainer.hpp"
#include "fs/directory.hpp"
#include "fs/globiterator.hpp"
#include "fs/owner.hpp"
#include "fs/path.hpp"
#include "ftp/task/task.hpp"
#include "ftp/task/types.hpp"
#include "ftp/xdupe.hpp"
#include "logs/logs.hpp"
#include "main.hpp"
#include "stats/compile.hpp"
#include "stats/stat.hpp"
#include "stats/types.hpp"
#include "stats/util.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/tag.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/util.hpp"
#include "util/error.hpp"
#include "util/path/status.hpp"
#include "util/string.hpp"
#include "util/timepair.hpp"
#include "cmd/site/adduser.hpp"
#include "cmd/util.hpp"

namespace cmd { namespace site
{

void ADDIPCommand::Execute()
{
  if (args[0] == "ADDIP" && !acl::AllowSiteCmd(client.User(), "addip"))
  {
    if (args[1] != client.User().Name() ||
        !acl::AllowSiteCmd(client.User(), "addipown"))
    {
      if (!client.User().HasGadminGID(acl::NameToPrimaryGID(args[1])) ||
          !acl::AllowSiteCmd(client.User(), "addipgadmin"))
      {
        throw cmd::PermissionError();
      }
    }
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }
  
  std::ostringstream os;
  os << "Adding IPs to " << user->Name() << ":";
  
  acl::IPStrength strength;
  std::vector<std::string> deleted;
  for (auto it = args.begin() + 2; it != args.end(); ++it)
  {
    util::Error ok;
    
    if (it->find('@') == std::string::npos)
    {
      os << "\nIP " << *it << " not added. Must be in format ident@address.";
      continue;
    }

    if (!acl::SecureIP(client.User(), *it, strength))
    {
      os << "\nIP " << *it << " not added: Must contain " << strength.String() << ".";
      logs::Security("INSECUREIP", "'%1%' attempted to add insecure ip '%2%' to '%3%'", 
                     client.User().Name(), *it, user->Name());
      continue;
    }

    if (!user->AddIPMask(*it, &deleted))
    {
      os << "\nIP " << *it << " not added: Wider matching mask already exists.";
      continue;
    }
      
    os << "\nIP " << *it << " added successfully.";
    logs::Siteop(client.User().Name(), "added ip '%2%' to '%1%'", user->Name(), *it);
    for (const std::string& del : deleted)
    {
      os << "\nAuto-removed unncessary IP " << del << "!";
      logs::Siteop(client.User().Name(), "deleted ip '%2%' from '%1%'", user->Name(), del);
    }
  }

  os << "\nCommand finished.";
  
  control.Reply(ftp::CommandOkay, os.str());
  
} 

void CHGADMINCommand::Execute()
{
  acl::GroupID gid = acl::NameToGID(args[2]);
  if (gid == -1)
  {
    control.Format(ftp::ActionNotOkay, "Group %1% doesn't exist.", args[2]);
    return;
  }
  
  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Format(ftp::ActionNotOkay, "User %1% doesn't exist.", args[1]);
    return;
  }

  if (!user->HasGID(gid))
  {
    control.Format(ftp::ActionNotOkay, "User %1% is not a member of %2%.", args[1], args[2]);
    return;
  }
  
  bool added = user->ToggleGadminGID(gid);
  control.Format(ftp::CommandOkay, "Gadmin flag %1% %2% for %3%.",
                 added ? "added to" : "removed from", args[1], args[2]);
  if (added)
    logs::Siteop(client.User().Name(), "gave gadmin rights to '%1%' for '%2%'", user->Name(), args[2]);
  else
    logs::Siteop(client.User().Name(), "removed gadmin rights from '%1%' for '%2%'", user->Name(), args[2]);
}

void CHGRPCommand::Execute()
{
  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }

  std::function<void()> go;
  std::vector<acl::GroupID> gids;
  auto it = args.begin() + 3;

  if (args[2] == "-") go = [&]() { user->DelGIDs(gids); };
  else if (args[2] == "=") go = [&]() { user->SetGIDs(gids); };
  else if (args[2] == "+") go = [&]() { user->AddGIDs(gids); };
  else
  {
    go = [&]() { user->ToggleGIDs(gids); };
    --it;
  }
  
  if (it == args.end()) throw cmd::SyntaxError();

  for (; it != args.end(); ++it)
  {
    if (*it == "default")
    {
      control.Reply(ftp::ActionNotOkay, "The default template group cannot have members.");
      return;
    }
    
    auto gid = acl::NameToGID(*it);
    if (gid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "Group " + *it + " doesn't exist.");
      return;
    }
    gids.emplace_back(gid);
  }
  
  go();
  
  if (user->PrimaryGID() == -1)
    control.Reply(ftp::CommandOkay, "User " + user->Name() + " now has no groups.");
  else
    control.Reply(ftp::CommandOkay, "User " + user->Name() + " now has groups: " + acl::GroupString(*user));
  
  logs::Siteop(client.User().Name(), "changed groups for '%1%' to '%2%'", user->Name(), acl::GroupString(*user));
}

void CHPASSCommand::Execute()
{
  if (args[0] == "CHPASS" && 
      !acl::AllowSiteCmd(client.User(), "chpass") &&
      acl::AllowSiteCmd(client.User(), "chpassgadmin") &&
      !client.User().HasGadminGID(acl::NameToPrimaryGID(args[1])))
  {
    throw cmd::PermissionError();
  }
  
  acl::PasswdStrength strength;  
  if (!acl::SecurePass(client.User(), args[2], strength))
  {
    std::ostringstream os;
    os << "Password not strong enough. Must meet the following minimum criteria:\n"
       << strength.String() << ".";
    control.Reply(ftp::ActionNotOkay, os.str());
    return;
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }
  
  user->SetPassword(args[2]);
  control.Reply(ftp::CommandOkay, "Password changed.");
  logs::Siteop(client.User().Name(), "changed password for '%1%'", user->Name());
}

void DELIPCommand::Execute()
{
  if (!acl::AllowSiteCmd(client.User(), "delip"))
  {
    if (args[1] != client.User().Name() ||
        !acl::AllowSiteCmd(client.User(), "delipown"))
    {
      if (!client.User().HasGadminGID(acl::NameToPrimaryGID(args[1])) ||
          !acl::AllowSiteCmd(client.User(), "delipgadmin"))
      {
        throw cmd::PermissionError();
      }
    }
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }

  std::ostringstream os;
  
  std::vector<int> indexes;
  bool all = false;
  const auto& masks = user->IPMasks();
  for (auto it = args.begin() + 2; it != args.end(); ++it)
  {
    if (*it == "*")
    {
      all = true;
      indexes.clear();
      break;
    }
    
    try
    {
      int index = boost::lexical_cast<int>(*it);
      if (index < 0 || index > static_cast<ssize_t>(masks.size()) - 1)
      {
        control.Reply(ftp::ActionNotOkay, "IP mask index out of range.");
        return;
      }
      
      indexes.emplace_back(index);
    }
    catch (const std::bad_cast&)
    {
      auto it2 = std::find(masks.begin(), masks.end(), *it);
      if (it2 == user->IPMasks().end())
      {
        control.Reply(ftp::ActionNotOkay, "No IP mask matching " + *it + " exists for " + args[1] + ".");
        return;
      }
      indexes.emplace_back(std::distance(masks.begin(), it2));
    }
  }

  os << "Deleting IPs from " << user->Name();
  if (all)
  {
    std::vector<std::string> deleted = user->IPMasks();
    user->ClearIPMasks();
    for (const std::string& mask : deleted)
    {
      os << "\nIP " << mask << " deleted successfully.";
      logs::Siteop(client.User().Name(), "deleted ip '%2%' from '%1%'", user->Name(), mask);
    }
  }
  else
  {
    std::sort(indexes.begin(), indexes.end(), std::greater<int>());
    std::string mask;
    for (int index : indexes)
    {
      std::string mask = user->DelIPMask(index);
      os << "\nIP " << mask << " deleted successfully.";
      logs::Siteop(client.User().Name(), "deleted ip '%2%' from '%1%'", user->Name(), mask);
    }
  }

  os << "\nCommand finished.";
  
  control.Reply(ftp::CommandOkay, os.str());
}

void DELUSERCommand::Execute()
{
  if (!acl::AllowSiteCmd(client.User(), "deluser") &&
      acl::AllowSiteCmd(client.User(), "delusergadmin") &&
      !client.User().HasGadminGID(acl::NameToPrimaryGID(args[1])))
  {
    throw cmd::PermissionError();
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    throw cmd::NoPostScriptError();    
  }
  
  if (user->HasFlag(acl::Flag::Siteop) && !cfg::Get().IsMaster(client.User().Name()))
  {
    control.Reply(ftp::ActionNotOkay, "Only masters can delete other siteops.");
    throw cmd::NoPostScriptError();    
  }
  
  if (user->Name() == "default" || user->ID() == 0 || user->ID() == 1)
  {
    control.Reply(ftp::ActionNotOkay, "That user is required by the server so cannot be deleted.");
    throw cmd::NoPostScriptError();    
  }

  if (user->HasFlag(acl::Flag::Deleted))
  {
    control.Reply(ftp::ActionNotOkay, "User " + user->Name() + " is already deleted.");
    throw cmd::NoPostScriptError();    
  }
  
  user->AddFlag(acl::Flag::Deleted);

  std::future<int> future;
  std::make_shared<ftp::task::KickUser>(user->ID(), future)->Push();
  
  future.wait();
  int kicked = future.get();
  std::ostringstream os;
  os << "User " << args[1] << " has been deleted.";
  if (kicked) os << " (" << kicked << " login(s) kicked)";

  control.Reply(ftp::CommandOkay, os.str());
  logs::Siteop(client.User().Name(), "deleted user '%1%'", user->Name());
}

void DISKFREECommand::Execute()
{
  std::string pathStr = argStr.empty() ? "." : argStr;
  fs::VirtualPath path(fs::PathFromUser(pathStr));
  if (!acl::path::DirAllowed<acl::path::View>(client.User(), path))
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ":" + util::Error::Failure(EACCES).Message());
    return;
  }
  
  unsigned long long bytes;
  auto e = util::path::FreeDiskSpace(fs::MakeReal(path).ToString(), bytes);
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ":" + e.Message());
    return;
  }
  
  std::ostringstream os;
  os << "Disk free: " << util::ToString(bytes / 1024 / 1024.0, 2) << "MB";
  control.Reply(ftp::CommandOkay, os.str());
}

void DUPECommand::Execute()
{
  int number = 10;
  unsigned termsOffset = 1;
  if (util::ToLowerCopy(args[1]) == "-max")
  {
    if (args.size() < 4) throw cmd::SyntaxError();
    
    try
    {
      number = util::StrToInt(args[2]);
      if (number <= 0) throw std::bad_cast();
    }
    catch (const std::bad_cast&)
    {
      throw cmd::SyntaxError();
    }
    
    termsOffset += 2;
  }

  std::vector<std::string> terms(args.begin() + termsOffset, args.end());
  auto results = db::dupe::Search(terms, number);
  if (results.empty()) control.Reply(ftp::CommandOkay, "No dupe results.");
  else
  {
    boost::optional<text::Template> templ;
    try
    {
      templ.reset(text::Factory::GetTemplate("dupe"));
    }
    catch (const text::TemplateError& e)
    {
      control.Reply(ftp::ActionNotOkay, e.Message());
      return;
    }
    
    std::ostringstream os;
    os << templ->Head().Compile();

    text::TemplateSection& body = templ->Body();

    unsigned index = 0;
    for (const auto& result : results)
    {
      body.RegisterValue("index", ++index);
      body.RegisterValue("date", boost::posix_time::to_iso_string(result.dateTime).substr(0, 8));
      body.RegisterValue("directory", result.directory);
      body.RegisterValue("section", result.section);
      os << body.Compile();
    }

    text::TemplateSection& foot = templ->Foot();
    foot.RegisterValue("count", results.size());
    os << foot.Compile();
    
    control.Reply(ftp::CommandOkay, os.str());
  }
}

std::string HasFlag(const acl::User& user, const acl::Flag& flag)
{
  return user.HasFlag(flag) ? "*" : " ";
}

void FLAGSCommand::Execute()
{
  if (args.size() == 2 && args[1] != client.User().Name() &&
      !acl::AllowSiteCmd(client.User(), "flags"))
  {
    throw cmd::PermissionError();
  }

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("flags"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  boost::optional<acl::User> user(client.User());
  if (args.size() == 2)
  {
    user = acl::User::Load(args[1]);
    if (!user)
    {
      control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
      return;
    }
  }
  
  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();
  
  body.RegisterValue("user", user->Name());
  body.RegisterValue("flag1", HasFlag(*user, acl::Flag::Siteop));
  body.RegisterValue("flag2", HasFlag(*user, acl::Flag::Gadmin));
  body.RegisterValue("flag3", HasFlag(*user, acl::Flag::Template));
  body.RegisterValue("flag4", HasFlag(*user, acl::Flag::Exempt));
  body.RegisterValue("flag5", HasFlag(*user, acl::Flag::Color));
  body.RegisterValue("flag6", HasFlag(*user, acl::Flag::Deleted));
  body.RegisterValue("flag7", HasFlag(*user, acl::Flag::Useredit));
  body.RegisterValue("flag8", HasFlag(*user, acl::Flag::Anonymous));

  body.RegisterValue("flaga", HasFlag(*user, acl::Flag::Nuke));
  body.RegisterValue("flagb", HasFlag(*user, acl::Flag::Unnuke));
  body.RegisterValue("flagc", HasFlag(*user, acl::Flag::Undupe));
  body.RegisterValue("flagd", HasFlag(*user, acl::Flag::Kick));
  body.RegisterValue("flage", HasFlag(*user, acl::Flag::Kill));
  body.RegisterValue("flagf", HasFlag(*user, acl::Flag::Take));
  body.RegisterValue("flagg", HasFlag(*user, acl::Flag::Give));
  body.RegisterValue("flagh", HasFlag(*user, acl::Flag::Users));
  body.RegisterValue("flagi", HasFlag(*user, acl::Flag::Idler));
  body.RegisterValue("flagj", HasFlag(*user, acl::Flag::Custom1));
  body.RegisterValue("flagk", HasFlag(*user, acl::Flag::Custom2));
  body.RegisterValue("flagl", HasFlag(*user, acl::Flag::Custom3));
  body.RegisterValue("flagm", HasFlag(*user, acl::Flag::Custom4));
  body.RegisterValue("flagn", HasFlag(*user, acl::Flag::Custom5));

  std::ostringstream os;
  os << head.Compile();
  os << body.Compile();
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}

void GADDUSERCommand::Execute()
{
  bool gadmin = false;
  if (!acl::AllowSiteCmd(client.User(), "gadduser") &&
       acl::AllowSiteCmd(client.User(), "gaddusergadmin"))
  {
    auto gid = acl::NameToGID(args[1]);
    if (!client.User().HasGadminGID(gid)) throw cmd::PermissionError();
    gadmin = true;
  }

  std::string cpArgStr("GADDUSER ");
  cpArgStr += args[2];
  for (auto it = args.begin() + 3; it != args.end(); ++it)
    cpArgStr += " " + *it;

  std::vector<std::string> cpArgs;
  util::Split(cpArgs, cpArgStr, " ");
  ADDUSERCommand(client, cpArgStr, cpArgs).Execute(args[1], gadmin);
}

void GIVECommand::Execute()
{
  std::string section;
  
  if (util::ToLowerCopy(args[1]) == "-s")
  {
    section = util::ToUpperCopy(args[2]);
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

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }

  long long credits;
  try
  {
    credits = cfg::ParseSize(args[2]);
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }

  bool own = false;
  std::ostringstream os;
  if (acl::AllowSiteCmd(client.User(), "giveown") &&
      !acl::AllowSiteCmd(client.User(), "give"))
  {
    int ratio = client.User().SectionRatio(section);
    if (ratio == 0 || (ratio == -1 && client.User().DefaultRatio() == 0))
    {
      control.Reply(ftp::ActionNotOkay, "Not allowed to give credits when you have leech!");
      return;
    }

    // take away users credits/warn them
    
    if (!client.User().DecrSectionCredits(section, credits))
    {
      control.Reply(ftp::ActionNotOkay, "Not enough credits to do that.");
      return;
    }
    
    os << "Taken " << credits << "KB credits from you!\n";
    own = true;
  }
  
  // give user the credits
  user->IncrSectionCredits(section, credits);
  os << "Given " << util::ToString(credits / 1024.0, 2) << "MB credits to " << user->Name();
  if (!section.empty()) os << " on section " << section;
  os << ".";
  control.Reply(ftp::CommandOkay, os.str());
  
  if (!own)
  {
    if (section.empty())
      logs::Siteop(client.User().Name(), "gave '%2%' credits to '%1%'", user->Name(), credits);
    else
      logs::Siteop(client.User().Name(), "gave '%2%' credits to '%1%' on section '%3%'", 
                   user->Name(), credits, section);
  }
}

void GOODBYECommand::Execute()
{
  fs::Path goodbyePath(acl::message::Choose<acl::message::Goodbye>(client.User()));
  if (!goodbyePath.IsEmpty())
  {
    std::string goodbye;
    auto e = text::GenericTemplate(client, goodbyePath, goodbye);
    if (!e)
    {
      logs::Error("Failed to display goodbye message: %1%", e.Message());
    }
    else
    {
      control.Reply(ftp::CommandOkay, goodbye);
      return;
    }
  }
  
  control.Reply(ftp::CommandOkay, "No goodbye message");
}

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
      number = util::StrToInt(args[4]);
      if (number < 0) throw std::bad_cast();
    }
    catch (const std::bad_cast&)
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

void GROUPSCommand::Execute()
{
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("groups"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  auto groups = acl::Group::GetGroups();

  std::ostringstream os;
  text::TemplateSection& head = templ->Head();
  os << head.Compile();

  text::TemplateSection& body = templ->Body();

  for (auto& group: groups)
  {    
    body.RegisterValue("users", group.NumMembers());
    body.RegisterValue("group", group.Name());
    body.RegisterValue("descr", group.Description());
    os << body.Compile();
  }

  text::TemplateSection& foot = templ->Foot();
  foot.RegisterValue("total_groups", groups.size());
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}

void GRPADDCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Groupname, args[1]))
  {
    control.Reply(ftp::ActionNotOkay, "Groupname contains invalid characters");
    return;
  }
  
  if (args.size() > 2)
  {
    argStr.erase(0, args[1].length());
    util::Trim(argStr);

    if (!acl::Validate(acl::ValidationType::Tagline, argStr))
    {
      control.Reply(ftp::ActionNotOkay, "Description contains invalid characters");
      return;
    }
  }
  
  auto templateGroup = acl::Group::Load("default");
  if (!templateGroup)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to load default group template.");
    return;
  }

  auto group = acl::Group::FromTemplate(args[1], *templateGroup);
  if (!group)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[1] + " alread exists.");
    return;
  }
  
  if (args.size() > 2) group->SetDescription(argStr);  
  control.Reply(ftp::CommandOkay, "Group " + args[1] + " successfully added.");
  logs::Siteop(client.User().Name(), "added group '%1%' with description '%2%'", group->Name(), group->Description());
}

void GRPDELCommand::Execute()
{
  auto group = acl::Group::Load(args[1]);
  if (!group)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[1] + " doesn't exist.");
    return;
  }

  if (group->Name() == "default" || group->ID() == 1 || group->ID() == 0)
  {
    control.Reply(ftp::ActionNotOkay, "That group is required by the server so cannot be deleted.");
    return;
  }
  
  auto numMembers = group->NumMembers();
  if (numMembers < 0)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to determine number of memmbers.");
    return;
  }
  
  if (numMembers > 0)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to delete group with members.");
    return;
  }

  group->Purge();
  control.Reply(ftp::CommandOkay, "Group " + args[1] + " deleted.");
  logs::Siteop(client.User().Name(), "deleted group '%1%'", args[1]);
}

void GRPRENCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Groupname, args[1]))
  {
    control.Reply(ftp::ActionNotOkay, "Groupname contains invalid characters");
    return;
  }

  auto group = acl::Group::Load(args[1]);
  if (!group)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[1] + " doesn't exist.");
    return;
  }
  
  if (group->Name() == "default")
  {
    control.Reply(ftp::ActionNotOkay, "Cannot rename default template group.");
    return;
  }

  if (!group->Rename(args[2]))
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[2] + " already exists.");
    return;
  }
  
  control.Reply(ftp::CommandOkay, "Group " + args[1] + " renamed to: " + args[2]);
  logs::Siteop(client.User().Name(), "renamed group '%1% to '%2%'", args[1], args[2]);
}

void IDLECommand::Execute()
{
  namespace pt = boost::posix_time;

  if (client.User().IdleTime() == 0)
  {
    control.Reply(ftp::CommandOkay, "This command doesn't apply to you, you have no idle limit.");
    return;
  }

  if (args.size() == 1)
  {
    std::ostringstream os;
    os << "Your current idle timeout is "
       << client.IdleTimeout().total_seconds() << " seconds.";
    control.Reply(ftp::CommandOkay, os.str());
  }                
  else
  {
    try
    {
      pt::seconds idleTimeout(util::StrToLong(args[1]));
    
      const cfg::Config& config = cfg::Get();
      
      pt::seconds maximum(config.IdleTimeout().Maximum());
      if (client.User().IdleTime() > 0)
        maximum = pt::seconds(client.User().IdleTime());
      
      if (idleTimeout < config.IdleTimeout().Minimum() ||
          idleTimeout > maximum)
      {
        std::ostringstream os;
        os << "Idle timeout must be between " 
           << config.IdleTimeout().Minimum().total_seconds()
           << " and " << maximum.total_seconds() << " seconds.";
        control.Reply(ftp::SyntaxError, os.str());
        return;
      }
      
      client.SetIdleTimeout(idleTimeout);
      std::ostringstream os;
      os << "Idle timeout set to " << idleTimeout.total_seconds() 
         << " seconds.";
      control.Reply(ftp::CommandOkay, os.str());
    }
    catch (const std::bad_cast&)
    {
      throw cmd::SyntaxError();
    }
  }
}

void KICKCommand::Execute()
{
  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    throw cmd::NoPostScriptError();
  }

  if (user->HasFlag(acl::Flag::Siteop) && user->ID() != client.User().ID())
  {
    control.Reply(ftp::ActionNotOkay, "Cannot kick a siteop.");
    throw cmd::NoPostScriptError();
  } 

  std::future<int> future;
  std::make_shared<ftp::task::KickUser>(user->ID(), future)->Push();
  int kicked = future.get();
  if (kicked == 0)
  {
    control.Format(ftp::CommandOkay, "User %1% is not online.", args[1]);
  }
  else
  {
    control.Format(ftp::CommandOkay, "Kicked %1% (%2% login%3%).", 
                   args[1], kicked, kicked == 1 ? "" : "s");
  }
  
  if (kicked == 0) throw cmd::NoPostScriptError();
  logs::Siteop(client.User().Name(), "kicked '%1%', '%2%' logins", user->Name(), kicked);
}

void NEWCommand::Execute()
{
  int number = 10;
  if (args.size() == 2)
  {
    try
    {
      number = util::StrToInt(args[1]);
      if (number <= 0) throw std::bad_cast();
    }
    catch (const std::bad_cast&)
    {
      throw cmd::SyntaxError();
    }
  }

  auto results = db::index::Newest(number);

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("new"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  std::ostringstream os;
  os << templ->Head().Compile();

  text::TemplateSection& body = templ->Body();

  auto now = boost::posix_time::second_clock::local_time();
  unsigned index = 0;
  for (const auto& result : results)
  {
    auto real = fs::MakeReal(fs::VirtualPath(result.path));
    long long kBytes;
    auto e = fs::DirectorySize(real, cfg::Get().DirSizeDepth(), kBytes, true);
    if (e.Errno() == ENOENT)
    {
      db::index::Delete(result.path);
      continue;
    }
    
    auto owner = fs::GetOwner(real);
    
    body.RegisterValue("index", ++index);
    body.RegisterValue("datetime", boost::posix_time::to_simple_string(result.dateTime));
    body.RegisterValue("age", Age(now - result.dateTime));
    body.RegisterValue("path", fs::Path(result.path).Basename().ToString());
    body.RegisterValue("section", result.section);
    body.RegisterSize("size", e ? kBytes : -1);
    body.RegisterValue("user", acl::UIDToName(owner.UID()));
    body.RegisterValue("group", acl::GIDToName(owner.GID()));
    os << body.Compile();
  }

  text::TemplateSection& foot = templ->Foot();
  foot.RegisterValue("count", results.size());
  os << foot.Compile();
  
  control.Reply(ftp::CommandOkay, os.str());
}

void PASSWDCommand::Execute()
{
  std::string cpArgStr("PASSWD ");
  cpArgStr += client.User().Name();
  cpArgStr += " ";
  cpArgStr += args[1];

  std::vector<std::string> cpArgs;
  util::Split(cpArgs, cpArgStr, " ");
  
  CHPASSCommand(client, cpArgStr, cpArgs).Execute();
}

void PURGECommand::PurgeAll()
{
  auto users = acl::User::GetUsers("*");
  users.erase(std::remove_if(users.begin(), users.end(),
          [](const acl::User& user)
          {
            return !user.HasFlag(acl::Flag::Deleted);
          }), users.end());
  
  if (users.empty())
  {
    control.Reply(ftp::ActionNotOkay, "No deleted users to purge.");
    return;
  }
  
  for (auto& user : users)
  {
    user.Purge();
    logs::Siteop(client.User().Name(), "purged user '%1%'", user.Name());
  }
  
  if (users.size() == 1)
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been purged.");
  else
    control.Format(ftp::CommandOkay, "Purged %1% users.", users.size());
}

void PURGECommand::Execute()
{
  if (args[1] == "*")
  {
    PurgeAll();
    return;
  }
  
  auto user = acl::User::Load(args[1]);
  if (!user)
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
  else
  if (!user->HasFlag(acl::Flag::Deleted))
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " is not deleted.");
  else
  {
    user->Purge();
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been purged.");
    logs::Siteop(client.User().Name(), "purged user '%1%'", user->Name());
  }
}

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
      number = util::StrToInt(args[4]);
      if (number < 0) throw std::bad_cast();
    }
    catch (const std::bad_cast&)
    {
      throw cmd::SyntaxError();
    }
  }

  int maxNumber = acl::stats::MaxUsers(client.User());
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

void READDCommand::Execute()
{
  bool gadmin = false;
  if (!acl::AllowSiteCmd(client.User(), "readd") &&
      acl::AllowSiteCmd(client.User(), "readdgadmin"))
  {
    if (!client.User().HasGadminGID(acl::NameToPrimaryGID(args[1]))) throw cmd::PermissionError();  
    gadmin = true;
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }
  
  if (!user->HasFlag(acl::Flag::Deleted))
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " is not deleted.");
    return;
  }
  
  if (gadmin)
  {
    auto group = acl::Group::Load(user->PrimaryGID());
    if (!group)
    {
      control.Reply(ftp::ActionNotOkay, "Unable to load group " + user->PrimaryGroup() + ".");
      return;
    }
    
    if (group->NumSlotsUsed() >= group->Slots())
    {
      control.Reply(ftp::ActionNotOkay, "Maximum number of slots exceeded for group " + group->Name() + ".");
      return;
    }
  }
  
  user->DelFlag(acl::Flag::Deleted);
  control.Reply(ftp::CommandOkay, "User " + args[1] + " has been readded.");
  logs::Siteop(client.User().Name(), "readded '%1%'", user->Name());
}

void RELOADCommand::Execute()
{
  typedef ftp::task::ReloadConfig::Result Result;
  std::future<std::pair<Result, Result>> future;
  std::make_shared<ftp::task::ReloadConfig>(future)->Push();
  
  future.wait();
  
  std::stringstream os;
  
  const auto result = future.get();
  auto& configResult = result.first;
  auto& templatesResult = result.second;

  bool furtherDetails = false;
  if (configResult == Result::Fail)
  {
    os << "Config failed to reload.";
    furtherDetails = true;
  }
  else
    os << "Config reloaded.";
  
  if (configResult == Result::StopStart)
  {
    os << "\nSome of the options changed require a full stop start.";
    furtherDetails = true;
  }
    
  if (templatesResult == Result::Fail)
  {
    os << "\nTemplates failed to reload.";
    furtherDetails = true;
  }
  else
    os << "\nTemplates reloaded.";

  if (furtherDetails)
  {
    os << "\nSee SITE LOGS ERROR for further details.";    
  }
  
  control.Reply(ftp::CommandOkay, os.str());
}

void RENUSERCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Username, args[2]))
  {
    control.Reply(ftp::ActionNotOkay, "Username contains invalid characters");
    return;
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }
  
  if (user->HasFlag(acl::Flag::Siteop) && !cfg::Get().IsMaster(client.User().Name()))
  {
    control.Reply(ftp::ActionNotOkay, "Only masters can rename other siteops.");
    return;
  }
  
  if (user->Name() == "default")
  {
    control.Reply(ftp::ActionNotOkay, "Cannot rename default template user.");
    return;
  }
  
  if (!user->Rename(args[2]))
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[2] + " already exists.");
    return;
  }
  
  control.Reply(ftp::CommandOkay, "User " + args[1] + " renamed to " + args[2] + ".");
  logs::Siteop(client.User().Name(), "RENUSER", args[1], args[2]);
}

void SEARCHCommand::Execute()
{
  int number = 10;
  unsigned termsOffset = 1;
  if (util::ToLowerCopy(args[1]) == "-max")
  {
    if (args.size() < 4) throw cmd::SyntaxError();
    
    try
    {
      number = util::StrToInt(args[2]);
      if (number <= 0) throw std::bad_cast();
    }
    catch (const std::bad_cast&)
    {
      throw cmd::SyntaxError();
    }
    
    termsOffset += 2;
  }

  std::vector<std::string> terms(args.begin() + termsOffset, args.end());
  auto results = db::index::Search(terms, number);
  if (results.empty()) control.Reply(ftp::CommandOkay, "No search results.");
  else
  {
    boost::optional<text::Template> templ;
    try
    {
      templ.reset(text::Factory::GetTemplate("search"));
    }
    catch (const text::TemplateError& e)
    {
      control.Reply(ftp::ActionNotOkay, e.Message());
      return;
    }
    
    std::ostringstream os;
    os << templ->Head().Compile();

    text::TemplateSection& body = templ->Body();

    unsigned index = 0;
    for (const auto& result : results)
    {
      long long kBytes;
      auto e = fs::DirectorySize(fs::MakeReal(fs::VirtualPath(result.path)),
                                 cfg::Get().DirSizeDepth(), kBytes, true);
      if (e.Errno() == ENOENT)
      {
        db::index::Delete(result.path);
        continue;
      }

      body.RegisterValue("index", ++index);
      body.RegisterValue("datetime", boost::posix_time::to_simple_string(result.dateTime));
      body.RegisterValue("path", result.path);
      body.RegisterValue("section", result.section);
      body.RegisterSize("size", e ? kBytes : -1);
      os << body.Compile();
    }

    text::TemplateSection& foot = templ->Foot();
    foot.RegisterValue("count", results.size());
    os << foot.Compile();
    
    control.Reply(ftp::CommandOkay, os.str());
  }
}

void SEENCommand::Execute()
{
  std::ostringstream os;
  if (args[1] == client.User().Name())
    os << "Looking at you right now!";
  else
  {
    auto user = acl::User::Load(args[1]);
    if (!user)
    {
      control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
      return;
    }

    if (!user->LastLogin())
      os << "User " << args[1] << " has never logged in.";
    else
      os << "Last saw " << args[1] << " on " << *user->LastLogin();
  }
      
  control.Reply(ftp::CommandOkay, os.str());
}

void SETPGRPCommand::Execute()
{
  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }

  auto gid = acl::NameToGID(args[2]);
  if (gid < 0)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[2] + " doesn't exist.");
    return;
  }
  
  if (args[2] == "default")
  {
    control.Reply(ftp::ActionNotOkay, "The default template group cannot have members.");
    return;
  }
  
  acl::GroupID oldGID = user->PrimaryGID();
  if (oldGID == gid)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[2] + " is already primary for " + args[1] + ".");
    return;
  }
  
  user->SetPrimaryGID(gid);
  std::ostringstream os;
  if (oldGID != -1) os << "Moved old primary group " << acl::GIDToName(oldGID) << " to secondary.\n";
  os << "Set primary group for " << args[1] << " to: " << args[2];
  control.Reply(ftp::CommandOkay, os.str());
  logs::Siteop(client.User().Name(), "set primary group for '%1%' to '%2%'", user->Name(), user->PrimaryGroup());
}

void SHUTDOWNCommand::Execute()
{
  util::ToLower(args[1]);
  if (args[1] == "siteop")
  {
    if (!ftp::Client::SetSiteopOnly()) 
      control.Reply(ftp::ActionNotOkay, "Server already shutdown to siteop only.");
    else
    {
      control.Reply(ftp::CommandOkay, "Server shutdown to siteop only.");
      logs::Siteop(client.User().Name(), "put the server into siteop only mode");
    }
  }
  else
  if (args[1] == "reopen")
  {
    if (!ftp::Client::SetReopen())
      control.Reply(ftp::ActionNotOkay, "Server already open for all users.");
    else
    {
      control.Reply(ftp::CommandOkay, "Server reopened for all users.");
      logs::Siteop(client.User().Name(), "reopened the server for all users");
    }
  }
  else
  if (args[1] == "full")
  {
    control.Reply(ftp::ServiceUnavailable, "Full server shutdown/exit in progress..");
    logs::Siteop(client.User().Name(), "shut the server down");
    std::make_shared<ftp::task::Exit>()->Push();
  }
  else
    throw cmd::SyntaxError();
}

void SREPLYCommand::Execute()
{
  if (args.size() == 1)
  {
    control.Reply(ftp::CommandOkay, "Single line reply mode is currently " +
          std::string(control.SingleLineReplies() ? "on" : "off") + ".");
  }                
  else
  {
    util::ToLower(args[1]);
    if (args[1] == "on")
    {
      control.SetSingleLineReplies(true);
      control.Reply(ftp::SyntaxError, "Single line reply mode now set to on.");
    }
    else if (args[1] == "off")
    {
      control.SetSingleLineReplies(false);
      control.Reply(ftp::SyntaxError, "Single line reply mode now set to off.");
    }
    else
      throw cmd::SyntaxError();
  }
}

void STATCommand::Execute()
{
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("stat"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  text::RegisterGlobals(client, templ->Body());  
  control.Reply(ftp::CommandOkay, templ->Body().Compile(true));
}

void STATSCommand::Execute()
{
  if (args.size() == 2 && args[1] != client.User().Name() && 
      !acl::AllowSiteCmd(client.User(), "stats"))
  {
    throw cmd::PermissionError();
  }
  
  std::string userName = args.size() == 2 ? args[1] : client.User().Name();
  
  auto uid = acl::NameToUID(userName);
  if (uid < 0)
  {
    control.Reply(ftp::ActionNotOkay, "User " + userName + " doesn't exist.");
    return;
  }

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("stats"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::ostringstream os;
  text::TemplateSection& head = templ->Head();
  head.RegisterValue("user", userName);
  os << head.Compile();
  
  std::map< ::stats::Timeframe, std::map< ::stats::Direction, ::stats::Stat>> totals;
  
  text::TemplateSection& body = templ->Body();
  for (const auto& kv : cfg::Get().Sections())
  {
    body.RegisterValue("section", kv.first);

    for (auto tf : ::stats::timeframes)
    {
      for (auto dir : ::stats::directions)
      {
        std::string prefix = util::EnumToString(tf) + "_" +
                             util::EnumToString(dir) + "_";
        auto stat = db::stats::CalculateSingleUser(uid, kv.first, tf, dir);
        body.RegisterValue(prefix + "files", stat.Files());
        body.RegisterSize(prefix + "size", stat.KBytes());
        body.RegisterSpeed(prefix + "speed", stat.Speed());
    
        totals[tf][dir].Incr(stat);
      }
    }

    os << body.Compile();
  }
  
  text::TemplateSection& foot = templ->Foot();

  for (auto tf : ::stats::timeframes)
  {
    for (auto dir : ::stats::directions)
    {
      std::string prefix = util::EnumToString(tf) + "_" +
                           util::EnumToString(dir) + "_";
      foot.RegisterValue(prefix + "files", totals[tf][dir].Files());
      foot.RegisterSize(prefix + "size", totals[tf][dir].KBytes());
      foot.RegisterSpeed(prefix + "speed", totals[tf][dir].Speed());
    }
  }

  os << foot.Compile();
  
  control.Reply(ftp::CommandOkay, os.str());
}

void SWHOCommand::Execute()
{
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("swho"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  control.Reply(ftp::CommandOkay, cmd::CompileWhosOnline(*templ));
}

void TADDUSERCommand::Execute()
{
  auto templateUser = acl::User::Load(args[1]);
  if (!templateUser)
  {
    control.Reply(ftp::ActionNotOkay, "Template user " + args[1] + " doesn't exist.");
    throw cmd::NoPostScriptError();
  }
  
  if (!templateUser->HasFlag(acl::Flag::Template))
  {
    control.Reply(ftp::ActionNotOkay, templateUser->Name() + " is not a template user.");
    throw cmd::NoPostScriptError();
  }
  
  std::string cpArgStr("TADDUSER ");
  cpArgStr += args[2];
  for (auto it = args.begin() + 3; it != args.end(); ++it)
    cpArgStr += " " + *it;

  std::vector<std::string> cpArgs;
  util::Split(cpArgs, cpArgStr, " ");
  ADDUSERCommand(client, cpArgStr, cpArgs).Execute(*templateUser);
}

void TAGLINECommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Tagline, argStr))
  {
    control.Reply(ftp::ActionNotOkay, "Tagline contains invalid characters");
    return;
  }

  client.User().SetTagline(argStr);
  control.Reply(ftp::CommandOkay, "New Tagline: " + argStr);

  logs::Event("TAGLINE", "user", client.User().Name(), 
              "group", client.User().PrimaryGroup(), 
              "tagline", client.User().Tagline());
}

void TAKECommand::Execute()
{
  std::string section;
  if (util::ToLowerCopy(args[1]) == "-s")
  {
    section = util::ToUpperCopy(args[2]);
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

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }

  long long credits;
  try
  {
    credits = cfg::ParseSize(args[2]); 
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
  
  user->DecrSectionCreditsForce(section, credits);
  
  std::ostringstream os;
  os << "Taken " << util::ToString(credits / 1024.0, 2) << "MB credits from " << user->Name();
  if (!section.empty()) os << " on section " << section;
  os << ".";
  control.Reply(ftp::CommandOkay, os.str());
  
  if (section.empty())
    logs::Siteop(client.User().Name(), "took '%2%' credits from '%1%'", user->Name(), credits);
  else
    logs::Siteop(client.User().Name(), "took '%2%' credits from '%1%' on section '%3%'", user->Name(), credits, section);
}

namespace pt = boost::posix_time;
std::string FormatDuration(const boost::posix_time::time_duration& duration)
{
  std::ostringstream os;
  if (duration.hours() / 60 > 0) os << static_cast<unsigned>(duration.hours() / 24) << "d ";
  if (duration.hours()) os << duration.hours() % 24 << "h ";
  if (duration.minutes()) os << duration.minutes() << "m ";
  if (duration.seconds()) os << duration.seconds() << "s";
  return os.str();
}

void TIMECommand::Execute()
{
  namespace gd = boost::gregorian;
  namespace lt = boost::local_time;
  
  auto now = pt::second_clock::local_time();
  time_t t = (now - pt::ptime(gd::date(1970, 1, 1))).total_seconds();
  struct tm tm;
  std::string timezone = lt::posix_time_zone(localtime_r(&t, &tm)->tm_zone).to_posix_string();
  
  std::ostringstream os;
  os << "Current time : " << now << " " << timezone << "\n"
     << "Logged in at : " << client.LoggedInAt() << " " << timezone << "\n"
     << "Time online  : " << FormatDuration(now - client.LoggedInAt()) << "\n"
     << "End of day   : " << FormatDuration(pt::ptime(now.date() + gd::date_duration(1)) - now) << "\n"
     << "End of week  : " << FormatDuration(pt::ptime(now.date() + gd::date_duration(7 - now.date().day_of_week())) - now) << "\n"   
     << "End of month : " << FormatDuration(pt::ptime(now.date().end_of_month() + gd::date_duration(1)) - now) << "\n"
     << "End of year  : " << FormatDuration(pt::ptime(gd::date(now.date().year() + 1, 1, 1)) - now);
  
  control.Reply(ftp::CommandOkay, os.str());
}

void TRAFFICCommand::Execute()
{
  std::map<stats::Timeframe, std::pair<long long, long long>> combined;
    
  boost::optional<text::Template> tmpl;
  try
  {
    tmpl.reset(text::Factory::GetTemplate("traffic"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
     
  std::vector<std::string> sections;
  for (auto& kv : cfg::Get().Sections())
  {
    sections.emplace_back(kv.first);
  }
  
  sections.emplace_back("");

  auto& body = tmpl->Body();
  
  std::ostringstream tos;
  for (auto& section : sections)
  {
    body.RegisterValue("section", section);
    for (auto tf : ::stats::timeframes)
    {
      db::stats::Traffic t(db::stats::TransfersTotal(tf, section));

      std::string prefix = util::EnumToString(tf) + "_";
      body.RegisterSize(prefix + "send", t.SendKBytes());
      body.RegisterSize(prefix + "receive", t.ReceiveKBytes());
      combined[tf].first += t.SendKBytes();
      combined[tf].second += t.ReceiveKBytes();    
    }
    tos << body.Compile();
  }
     
  auto& head = tmpl->Head();
  auto& foot = tmpl->Foot();

  for (auto tf : ::stats::timeframes)
  {
    db::stats::Traffic t(db::stats::ProtocolTotal(tf));

    std::string prefix = "protocol_" + util::EnumToString(tf) + "_";
    head.RegisterSize(prefix + "send", t.SendKBytes());
    head.RegisterSize(prefix + "receive", t.ReceiveKBytes());
    foot.RegisterSize(prefix + "send", t.SendKBytes());
    foot.RegisterSize(prefix + "receive", t.ReceiveKBytes());

    combined[tf].first += t.SendKBytes();
    combined[tf].second += t.ReceiveKBytes();
  }
  
  for (auto tf : ::stats::timeframes)
  {
    std::string prefix = "total_" + util::EnumToString(tf) + "_";
    head.RegisterSize(prefix + "send", combined[tf].first);
    head.RegisterSize(prefix + "receive", combined[tf].second);
    foot.RegisterSize(prefix + "send", combined[tf].first);
    foot.RegisterSize(prefix + "receive", combined[tf].second);
  }
  
  std::ostringstream os;
  os << head.Compile();
  os << tos.str();
  os << foot.Compile();
     
  //os << "`-----------'-----------'------------'------------'\n";
  
  control.Reply(ftp::CommandOkay, os.str());
}

void UPDATECommand::Execute()
{
  std::string pathStr = args.size() == 2 ? argStr : ".";
  auto pathMask = fs::PathFromUser(pathStr);
  try
  {
    using util::WildcardMatch;
  
    unsigned addedCount = 0;
    for (const auto& entry : fs::GlobContainer(client.User(), pathMask))
    {
      fs::VirtualPath entryPath(pathMask.Dirname() / entry);
      if (!cfg::Get().IsIndexed(entryPath.ToString())) continue;

      try
      {
        if (util::path::Status(fs::MakeReal(entryPath).ToString()).IsDirectory())
        {
          auto section = cfg::Get().SectionMatch(entryPath.ToString(), true);
          db::index::Add(entryPath.ToString(), section ? section->Name() : "");
          ++addedCount;
        }
      }
      catch (const util::SystemError&)
      { }
    }
    
    std::ostringstream os;
    os << addedCount << " entries added to the index";
    control.Reply(ftp::CommandOkay, os.str());
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ": " + e.Message());
  }
}

void USERCommand::Execute()
{
  std::string userName = args.size() == 2 ? args[1] : client.User().Name();
  if (!acl::AllowSiteCmd(client.User(), "user"))
  {
    if (userName != client.User().Name() ||
        !acl::AllowSiteCmd(client.User(), "userown"))
    {
      if (!client.User().HasGadminGID(acl::NameToPrimaryGID(userName)) ||
          !acl::AllowSiteCmd(client.User(), "usergadmin"))
      {
        throw cmd::PermissionError();
      }
    }
  }

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("user"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  auto user = acl::User::Load(userName);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + userName + " doesn't exist.");
    return;
  }

  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();

  body.RegisterValue("user", user->Name());
  body.RegisterValue("logged_in", user->LoggedIn());
  body.RegisterValue("created", boost::gregorian::to_simple_string(user->Created()));

  if (user->LastLogin())
  {
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration diff = now - *user->LastLogin();
    body.RegisterValue("last_login", boost::posix_time::to_simple_string(*user->LastLogin()));
    body.RegisterValue("since_login", boost::posix_time::to_simple_string(diff));
  }
  else
    body.RegisterValue("last_login", "Never");

  if (user->Expires())
    body.RegisterValue("expires", boost::gregorian::to_simple_string(*user->Expires()));
  else
    body.RegisterValue("expires", "Never");
    
  body.RegisterValue("creator", acl::UIDToName(user->Creator()));
  body.RegisterValue("flags", user->Flags());
  body.RegisterValue("ratio", acl::RatioString(*user));  
  body.RegisterValue("credits", acl::CreditString(*user));
  body.RegisterValue("groups", acl::GroupString(*user));
  body.RegisterValue("tagline", user->Tagline());
  body.RegisterValue("comment", user->Comment());
  body.RegisterValue("weekly_allot", acl::WeeklyAllotmentString(*user));

  const auto& ipMasks = user->IPMasks();
  for (auto it = ipMasks.begin(); it != ipMasks.end(); ++it)
  {
    std::ostringstream tag;
    tag << "ip" << std::distance(ipMasks.begin(), it);
    body.RegisterValue(tag.str(), *it);
  }

  std::ostringstream os;
  os << head.Compile();
  os << body.Compile();
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}


void USERSCommand::Execute()
{
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("users"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::string multiStr = args.size() == 2 ? args[1] : "*";
  auto users = acl::User::GetUsers(multiStr);

  std::ostringstream os;
  if (users.size() > 0)
  {
    text::TemplateSection& head = templ->Head();
    os << head.Compile();

    text::TemplateSection& body = templ->Body();

    long long totalUploaded = 0;
    long long totalDownloaded = 0;
    long long totalUploadedFiles = 0;
    long long totalDownloadedFiles = 0;

    for (auto& user: users)
    {
      ::stats::Stat upStat = db::stats::CalculateSingleUser(user.ID(), "", 
                            ::stats::Timeframe::Alltime, ::stats::Direction::Upload);
      ::stats::Stat dnStat = db::stats::CalculateSingleUser(user.ID(), "", 
                            ::stats::Timeframe::Alltime, ::stats::Direction::Download);

      body.RegisterValue("user", user.Name());
      body.RegisterValue("group", user.PrimaryGroup());
      body.RegisterSize("upload", upStat.KBytes());
      body.RegisterSize("download", dnStat.KBytes());

      totalUploaded += upStat.KBytes();
      totalDownloaded += dnStat.KBytes();
      totalUploadedFiles += upStat.Files();
      totalDownloadedFiles += dnStat.Files();
      
      os << body.Compile();
    }

    text::TemplateSection& foot = templ->Foot();
    foot.RegisterValue("total_users", users.size());
    foot.RegisterSize("total_uploaded", totalUploaded);
    foot.RegisterSize("total_downloaded", totalDownloaded);
    foot.RegisterValue("total_uploaded_files", totalUploadedFiles);
    foot.RegisterValue("total_downloaded_files", totalDownloadedFiles);
    
    os << foot.Compile();
    
    control.Reply(ftp::CommandOkay, os.str());
  }
  else
    control.Reply(ftp::ActionNotOkay, "Error, no users found.");

  return;    
}

void UTIMECommand::Style1(std::string& path, std::string& atime, std::string& mtime)
{
  boost::regex pattern("(.+) (\\d{14}) (\\d{14}) \\d{14} UTC", boost::regex::icase);
  boost::smatch match;
  if (!boost::regex_match(argStr, match, pattern)) throw cmd::SyntaxError();

  path = match[1].str();
  atime = match[2].str();
  mtime = match[3].str();
}

void UTIMECommand::Style2(std::string& path, std::string& atime, std::string& mtime)
{
  path = argStr.substr(args[1].length() + 1);
  util::Trim(path);
  atime = args[1];
  if (atime.length() != 14)
  {
    if (atime.length() != 12) throw cmd::SyntaxError();
    atime += "00";
  }
  mtime = atime;
}

void UTIMECommand::Execute()
{
  std::string pathStr;
  std::string atime;
  std::string mtime;
  
  try
  {
    Style1(pathStr, atime, mtime);
  }
  catch (const cmd::SyntaxError&)
  {
    Style2(pathStr, atime, mtime);
  }
  
  struct tm atm;
  if (!strptime(atime.c_str(), "%Y%m%d%H%M%S", &atm)) throw cmd::SyntaxError();

  struct tm mtm;
  if (!strptime(mtime.c_str(), "%Y%m%d%H%M%S", &mtm)) throw cmd::SyntaxError();
  
  auto path = fs::PathFromUser(pathStr);
  util::Error e(acl::path::FileAllowed<acl::path::Modify>(client.User(), path));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ": " + e.Message());
    return;
  }
  
  try
  {
    if (!util::path::Status(fs::MakeReal(path).ToString()).IsRegularFile())
    {
      control.Reply(ftp::ActionNotOkay, pathStr + ": Not a plain file.");
      return;
    }
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ": " + e.Message());
    return;
  }
  
  struct timeval tv[2] =  { { timegm(&atm), 0 }, { timegm(&mtm), 0 } };

  auto real(fs::MakeReal(path));
  if (utimes(real.CString(), tv))
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ": " + util::Error::Failure(errno).Message());
    return;
  }
  
  control.Reply(ftp::FileStatus, "UTIME OK");
}


void VERSCommand::Execute()
{
  control.Reply(ftp::CommandOkay, "This server is running: " + programFullname);
}

void WELCOMECommand::Execute()
{
  fs::Path welcomePath(acl::message::Choose<acl::message::Welcome>(client.User()));
  if (!welcomePath.IsEmpty())
  {
    std::string welcome;
    auto e = text::GenericTemplate(client, welcomePath, welcome);
    if (!e)
    {
      logs::Error("Failed to display welcome message: %1%", e.Message());
    }
    else
    {
      control.Reply(ftp::CommandOkay, welcome);
      return;
    }
  }
  
  control.Reply(ftp::CommandOkay, "No welcome message");
}

void WHOCommand::Execute()
{
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("who"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  control.Reply(ftp::CommandOkay, cmd::CompileWhosOnline(*templ));
}

void XDUPECommand::Execute()
{
  if (args.size() == 1)
  {
    if (client.XDupeMode() == ftp::xdupe::Mode::Disabled)
      control.Reply(ftp::CommandOkay, "Extended dupe mode is disabled.");
    else
    {
      std::ostringstream os;
      os << "Extended dupe mode " << static_cast<int>(client.XDupeMode())
         << " is enabled.";
      control.Reply(ftp::CommandOkay, os.str());
    }
    return;
  }
  
  int mode;
  try
  {
    mode = util::StrToInt(args[1]);
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
  
  if (mode < 0 || mode > 4)
  {
    throw cmd::SyntaxError();
  }
  
  client.SetXDupeMode(static_cast<ftp::xdupe::Mode>(mode));
  std::ostringstream os;
  os << "Activated extended dupe mode " << mode << ".";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
