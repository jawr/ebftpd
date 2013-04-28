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

#include <sstream>
#include <iomanip>
#include <cerrno>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include "cmd/site/nuking.hpp"
#include "cmd/error.hpp"
#include "db/nuking/nuking.hpp"
#include "text/factory.hpp"
#include "util/string.hpp"
#include "text/error.hpp"
#include "cmd/util.hpp"
#include "fs/path.hpp"
#include "acl/user.hpp"
#include "util/path/extattr.hpp"
#include "logs/logs.hpp"
#include "db/stats/stats.hpp"
#include "cfg/get.hpp"
#include "acl/path.hpp"
#include "cmd/arguments.hpp"
#include "util/path/status.hpp"
#include "util/path/dircontainer.hpp"
#include "util/path/recursivediriterator.hpp"
#include "fs/owner.hpp"
#include "fs/file.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace site
{

namespace
{

struct Nukee
{
  
  long long kBytes;
  int files;
  long long credits;
  
  Nukee() : kBytes(0), files(0), credits(0) { }
};

const char* nukeIdAttributeName = "user.ebftpd.nukeid";

}

std::string GetNukeID(const fs::RealPath& path)
{ 
  char buf[25];
  int len = getxattr(path.CString(), nukeIdAttributeName, buf, sizeof(buf));
  if (len < 0)
  {
    if (errno != ENODATA && errno != ENOATTR && errno != ENOENT)
    {
      logs::Error("Error while reading filesystem attribute %1%: %2%: %3%", 
                  nukeIdAttributeName, path, util::Error::Failure(errno).Message());
    }
    return "";
  }
  
  buf[len] = '\0';
  return buf;
}

void SetNukeID(const fs::RealPath& path, const std::string& id)
{
  if (setxattr(path.CString(), nukeIdAttributeName, id.c_str(), id.length(), 0) < 0)
  {
    logs::Error("Error while writing filesystem attribute %1%: %2%: %3%", 
                nukeIdAttributeName, path, util::Error::Failure(errno).Message());
  }
}

void RemoveNukeID(const fs::RealPath& path)
{
  if (removexattr(path.CString(), nukeIdAttributeName) > 0 &&
      errno != ENOENT && errno != ENODATA && errno != ENOATTR)
  {
    logs::Error("Error while removing filesystem attribute %1%: %2%: %3%", 
                nukeIdAttributeName, path, util::Error::Failure(errno).Message());
  }
}

fs::RealPath NukedPath(const fs::Path& path)
{
  const auto& config = cfg::Get();
  std::string nukedName(boost::replace_all_copy(config.NukedirStyle().Format(), "%D", 
                                                path.Basename().ToString()));
  return fs::MakeReal(path) / nukedName;
}

void RegisterHeadFoot(text::TemplateSection& ts, const db::nuking::Nuke& nuke, 
                      const fs::VirtualPath& path)
{
  ts.RegisterValue("path", path.ToString());
  ts.RegisterValue("directory", path.Basename().ToString());
  ts.RegisterValue("multiplier", std::to_string(nuke.Multiplier()) + 
                   (nuke.IsPercent() ? "%" : ""));
  ts.RegisterValue("reason", nuke.Reason());
  ts.RegisterValue("section", nuke.Section().empty() ? 
                   "NoSection" : nuke.Section());
}

boost::optional<db::nuking::Nuke> LookupNuke(const fs::VirtualPath& path, const fs::RealPath& nukedPath)
{
  boost::optional<db::nuking::Nuke> nuke;
  std::string id = GetNukeID(nukedPath);
  if (!id.empty()) nuke = db::nuking::LookupNukeByID(id);   
  if (!nuke)
  {
    nuke = db::nuking::LookupNukeByPath(path.ToString());
  }
  return nuke;
}

boost::optional<db::nuking::Nuke> LookupUnnuke(const fs::VirtualPath& path)
{
  boost::optional<db::nuking::Nuke> nuke;
  std::string id = GetNukeID(fs::MakeReal(path));
  if (!id.empty()) nuke = db::nuking::LookupUnnukeByID(id);   
  if (!nuke)
  {
    nuke = db::nuking::LookupUnnukeByPath(path.ToString());
  }
  return nuke;
}

std::map<acl::UserID, Nukee> CalculateNuke(const fs::RealPath& path, 
                                          long long& totalKBytes, time_t& modTime)
{
  using namespace util::path;
  std::map<acl::UserID, Nukee> nukees;
  
  try
  {
     modTime = Status(path.ToString()).ModTime();
    
    for (const std::string& entry : RecursiveDirContainer(path.ToString(), true))
    {
      if (Basename(entry).front() == '.') continue;
      
      Status status(entry);
      if (!status.IsRegularFile()) continue;
      
      auto& nukee = nukees[fs::GetOwner(entry).UID()];
      long long kBytes = status.Size() / 1024;
      nukee.kBytes += kBytes;
      nukee.files++;
      totalKBytes += kBytes;
    }
  }
  catch (const util::SystemError& e)
  {
    logs::Error("Error while nuking %1%: %2%", path, e.what());
    throw e;
  }
  
  return nukees;
}

void DeleteNukeContents(const fs::RealPath& path)
{
  using namespace util::path;

  try
  {
    // delete all files in directory recursively
    for (const std::string& entry : RecursiveDirContainer(path.ToString(), true))
    {
      if (IsRegularFile(entry))
      {
        auto e = fs::DeleteFile(fs::RealPath(entry));
        if (!e)
        {
          logs::Error("Unable to delete nuked file: %1%", entry);
        }
      }
    }

    // delete all sub directories recursively
    for (const std::string& entry : RecursiveDirContainer(path.ToString(), true))
    {
      if (IsDirectory(entry))
      {
        auto e = fs::RemoveDirectory(fs::RealPath(entry));
        if (!e)
        {
          logs::Error("Unable to delete nuked file: %1%", entry);
        }
      }
    }
  }
  catch (const util::SystemError& e)
  {
    logs::Error("Unable to read nuked directory contents: %1%", path);
  }
}

void DeleteNukeDir(const fs::RealPath& path)
{
  auto e = fs::RemoveDirectory(path);
  if (!e)
  {
    logs::Error("Unable to remove nuked directory: %1%", path);
  }
}

void RenameNukeDir(const fs::RealPath& path, const std::string& id)
{
  auto nukedPath = NukedPath(path);
  auto e = fs::RenameFile(path, nukedPath);
  if (!e)
  {
    logs::Error("Unable to rename nuked directory: %1% -> %2%: %3%", 
                path, nukedPath, e.Message());
    SetNukeID(path, id);
  }
  else
  {
    SetNukeID(nukedPath, id);
  }
}

void TakeStats(const std::map<acl::UserID, Nukee>& nukees, const std::string& section, time_t modTime)
{
  for (const auto& kv : nukees)
  {
    if (kv.second.kBytes > 0)
    {
      db::stats::UploadDecr(kv.first, kv.second.kBytes, modTime, 
                            section, kv.second.files);
    }
  }
}

void TakeCredits(const std::map<acl::UserID, Nukee>& nukees, 
                 const std::string& section, long long totalKBytes)
{
  const auto& config = cfg::Get();
  if (totalKBytes < config.NukedirStyle().EmptyKBytes())
  {
    // if we have small files in the dir, we penalise them with empty nuke
    if (!nukees.empty())
    {
      for (auto& kv : nukees)
      {
        auto user = acl::User::Load(kv.first);
        if (!user)
        {
          logs::Error("Unable to update user with uid %1% after nuke of: %2%", 
                      kv.first, path);
        }
        else
        {
          user->DecrSectionCreditsForce(creditSection, kv.second.credits);
        }
      }
    }
    else // otherwise we penalise the directory owner
    {
      auto owner = fs::GetOwner(real);
      auto& nukee = nukees[owner.UID()];
      nukee.credits = config.EmptyNuke();
      auto user = acl::User::Load(owner.UID());
      if (!user)
      {
        logs::Error("Unable to update user with uid %1% after nuke of: %2%", 
                    owner.UID(), path);
      }
      else
      {
        user->DecrSectionCreditsForce(creditSection, nukee.credits);
      }
    }
  }
  else
  {
    // calculate credit loss for each user and send database
    double percent = multiplier / 100.0;
    for (auto& kv : nukees)
    {
      auto user = acl::User::Load(kv.first);
      if (isPercent)
      {
        kv.second.credits = user->SectionCredits(creditSection) * percent;
        if (kv.second.credits < 0) kv.second.credits = 0;
      }
      else
      {
        kv.second.credits = kv.second.kBytes * multiplier;
      }

      if (!user)
      {
        logs::Error("Unable to update user with uid %1% after nuke of: %2%", 
                    kv.first, path);
      }
      else
      {
        user->DecrSectionCreditsForce(creditSection, kv.second.credits);
      }
    }
  }
}

db::nuking::Nuke Nuke(const fs::VirtualPath& path, int multiplier, bool isPercent, 
                      const std::string& reason)
{
  using namespace util::path;
  
  long long totalKBytes;
  time_t modTime;
  auto real = fs::MakeReal(path);
  std::map<acl::UserID, Nukee> nukees = CalculateNuke(real, totalKBytes, modTime);

  // loop over directory recursively, tallying up file count and size for each user
  
  const auto& config = cfg::Get();
  auto section = config.SectionMatch(path.ToString(), true);
  std::string creditSection(section && section->SeparateCredits() ? section->Name() : "");
  
  TakeCredits(nukees, section && section->SeparateCredits() ? section->Name() : "", totalKBytes);
  
  std::string statsSection(section ? section->Name() : "");
  TakeStats(nukees, statsSection, modTime);

  // convert Nukee to db serializable db::nuking::Nukee
  std::vector<db::nuking::Nukee> nukees2;
  for (const auto& kv : nukees)
  {
    nukees2.emplace_back(kv.first, kv.second.kBytes, 
                         kv.second.files, kv.second.credits);
  }
  
  auto action = config.NukedirStyle().GetAction();
  if (action != cfg::NukedirStyle::Keep)
  {
    DeleteNukeContents(real);
  }

  // remove old unnuke data if dir was unnuked in past
  auto unnuke = LookupUnnuke(path);
  if (unnuke) db::nuking::DelUnnuke(*unnuke);
  
  // prepare nuke data early so we can assign the nukd id to the directory
  auto nuke = db::nuking::Nuke(path.ToString(), statsSection, reason, 
                               multiplier, isPercent, modTime, nukees2);
  db::nuking::AddNuke(nuke);

  // finalise nuked directory
  if (action == cfg::NukedirStyle::DeleteAll)
  {
    DeleteNukeDir(real);
  }
  else
  {
    RenameNukeDir(real, nuke.ID());
  }

  return nuke;
}

void NUKECommand::Execute()
{
  auto args = cmd::ArgumentParser("path:s multi:s reason:m")(argStr);
  fs::VirtualPath path(fs::PathFromUser(args["path"]));
  
  const std::string& multi = args["multi"];
  const std::string& reason = args["reason"];
  
  bool isPercent = multi.back() == '%';
  int multiplier;
  try
  {
    multiplier = util::StrToInt(multi.substr(0, multi.length() - isPercent));
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
  
  const auto& config = cfg::Get();
  if ((config.MultiplierMax() != -1 && multiplier > cfg::Get().MultiplierMax()) ||
      multiplier <= 0)
  {
    control.Reply(ftp::ActionNotOkay, "Invalid nuke multiplier / percent.");
    return;
  }
  
  if (!acl::path::DirAllowed<acl::path::Nuke>(client.User(), path))
  {
    throw cmd::PermissionError();
  }

  boost::optional<db::nuking::Nuke> nuke;
  try
  {
    nuke = Nuke(path, multiplier, isPercent, reason);
  }
  catch (const util::RuntimeError& e)
  {
    control.Format(ftp::ActionNotOkay, "Error while nuking: %1%", e.what());
    return;
  }
  
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("nuke"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  std::ostringstream os;
  RegisterHeadFoot(templ->Head(), *nuke, path);
  RegisterHeadFoot(templ->Foot(), *nuke, path);
  
  os << templ->Head().Compile();

  auto& body = templ->Body();
  unsigned index = 0;
  long long totalKBytes = 0;
  long long totalCredits = 0;
  int totalFiles = 0;
  for (const auto& nukee : nuke->Nukees())
  {
    body.RegisterValue("index", ++index);
    body.RegisterValue("username", acl::UIDToName(nukee.UID()));
    body.RegisterSize("size", nukee.KBytes());
    body.RegisterSize("credits", nukee.Credits());
    body.RegisterValue("files", nukee.Files());
    os << body.Compile();
    
    totalKBytes += nukee.KBytes();
    totalCredits += nukee.Credits();
    totalFiles += nukee.Files();
  }

  auto& foot = templ->Foot();
  foot.RegisterValue("total_nukees", nuke->Nukees().size());
  foot.RegisterSize("total_size", totalKBytes);
  foot.RegisterSize("total_credits", totalCredits);
  foot.RegisterValue("total_files", totalFiles);
  os << foot.Compile();
  
  control.Reply(ftp::CommandOkay, os.str());
}

boost::optional<db::nuking::Nuke> Unnuke(const fs::VirtualPath& path, const std::string& reason)
{
  auto real = fs::MakeReal(path);
  auto nukedPath = NukedPath(real);
  auto nuke = LookupNuke(path, nukedPath);
  if (!nuke) return boost::none;
  
  const auto& config = cfg::Get();
  std::string creditSection;
  auto it = config.Sections().find(nuke->Section());
  if (it != config.Sections().end() && it->second.SeparateCredits())
  {
    creditSection = it->first;
  }
  
  // restore stats and credits to users
  for (const auto& nukee : nuke->Nukees())
  {
    auto user = acl::User::Load(nukee.UID());
    if (!user)
    {
      logs::Error("Unable to update user with uid %1% after unnuke of: %2%", 
                  nukee.UID(), path);
    }
    else
    {
      user->IncrSectionCredits(creditSection, nukee.Credits());
      db::stats::UploadIncr(nukee.UID(), nukee.KBytes(), nuke->ModTime(), 
                            nuke->Section(), nuke->Files());
    }
  }

  // remove old nuke data
  RemoveNukeID(nukedPath);
  db::nuking::DelNuke(*nuke);
  
  // restore directory to old name if it still exists
  auto e = fs::RenameFile(nukedPath, fs::MakeReal(path));
  if (!e && e.ValidErrno() && e.Errno() != ENOENT)
  {
    logs::Error("Unable to rename nuked directory: %1% -> %2%: %3%", 
                nukedPath, real, util::Error::Failure(errno).Message());
  }
  
  // insert new unnuke data
  nuke->Unnuke(reason);
  db::nuking::AddUnnuke(*nuke);
  SetNukeID(real, nuke->ID());
  
  return nuke;
}

void UNNUKECommand::Execute()
{
  auto args = cmd::ArgumentParser("path:s reason:m")(argStr);
  fs::VirtualPath path(fs::PathFromUser(args["path"]));
  
  const std::string& reason = args["reason"];
  
  if (!acl::path::DirAllowed<acl::path::Nuke>(client.User(), path))
  {
    throw cmd::PermissionError();
  }

  auto nuke = Unnuke(path, reason);
  if (!nuke)
  {
    control.Format(ftp::ActionNotOkay, 
                   "Unable to locate nuke data for directory: %1%", 
                   args["path"]);
    return;
  }
  
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("unnuke"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
    
  std::ostringstream os;
  RegisterHeadFoot(templ->Head(), *nuke, path);
  RegisterHeadFoot(templ->Foot(), *nuke, path);
  
  os << templ->Head().Compile();

  auto& body = templ->Body();
  unsigned index = 0;
  long long totalKBytes = 0;
  long long totalCredits = 0;
  int totalFiles = 0;
  for (const auto& nukee : nuke->Nukees())
  {
    body.RegisterValue("index", ++index);
    body.RegisterValue("username", acl::UIDToName(nukee.UID()));
    body.RegisterSize("size", nukee.KBytes());
    body.RegisterSize("credits", nukee.Credits());
    body.RegisterValue("files", nukee.Files());
    os << body.Compile();
    
    totalKBytes += nukee.KBytes();
    totalCredits += nukee.Credits();
    totalFiles += nukee.Files();
  }

  auto& foot = templ->Foot();
  foot.RegisterValue("total_nukees", nuke->Nukees().size());
  foot.RegisterSize("total_size", totalKBytes);
  foot.RegisterSize("total_credits", totalCredits);
  foot.RegisterValue("total_files", totalFiles);
  os << foot.Compile();
  
  control.Reply(ftp::CommandOkay, os.str());
}

std::string FormatNukees(const std::vector<db::nuking::Nukee>& nukees)
{
  std::ostringstream os;
  for (const auto& nukee : nukees)
  {
    if (!os.str().empty()) os << ", ";
    os << acl::UIDToName(nukee.UID()) 
       << std::setprecision(2) << std::fixed << (nukee.KBytes() / 1024.0) << "MB";
  }
  return os.str();
}

void NUKESCommand::Execute()
{
  int number = 10;
  if (args.size() == 2)
  {
    try
    {
      number = boost::lexical_cast<int>(args[1]);
      if (number <= 0) throw boost::bad_lexical_cast();
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw cmd::SyntaxError();
    }
  }
  auto nukes = isUnnukes ? db::nuking::NewestUnnukes(number) :
                           db::nuking::NewestNukes(number);
                           
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate(isUnnukes ? "unnukes" : "nukes"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  std::ostringstream os;
  os << templ->Head().Compile();

  auto& body = templ->Body();

  auto now = boost::posix_time::second_clock::local_time();
  unsigned index = 0;
  
  for (const auto& nuke : nukes)
  {
    body.RegisterValue("index", ++index);
    body.RegisterValue("datetime", boost::lexical_cast<std::string>(nuke.DateTime()));
    body.RegisterValue("modtime", boost::lexical_cast<std::string>(nuke.ModTime()));
    body.RegisterValue("age", Age(now - nuke.DateTime()));
    body.RegisterValue("files", nuke.Files());
    body.RegisterSize("size", nuke.KBytes());
    body.RegisterValue("directory", fs::Path(nuke.Path()).Basename().ToString());
    body.RegisterValue("path", nuke.Path());
    body.RegisterValue("section", nuke.Section());
    body.RegisterValue("nukees", FormatNukees(nuke.Nukees()));
    body.RegisterValue("reason", nuke.Reason());
    
    std::string multiplier(std::to_string(nuke.Multiplier()));
    if (nuke.IsPercent()) multiplier += '%';    
    else multiplier += 'x';
    
    body.RegisterValue("multi", multiplier);

    os << body.Compile();
  }

  auto& foot = templ->Foot();
  foot.RegisterValue("count", nukes.size());
  os << foot.Compile();
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
