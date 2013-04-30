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
#include "stats/util.hpp"

namespace cmd { namespace site
{

namespace
{
const char* nukeIdAttributeName = "user.ebftpd.nukeid";

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

fs::RealPath NukedPath(const fs::RealPath& path)
{
  const auto& config = cfg::Get();
  std::string nukedName(boost::replace_all_copy(config.NukeStyle().Format(), "%N", 
                                                path.Basename().ToString()));
  return path.Dirname() / nukedName;
}

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

std::string NukeTemplateCompile(const db::nuking::Nuke& nuke, text::Template& templ, const fs::VirtualPath& path)
{
  std::ostringstream os;
  RegisterHeadFoot(templ.Head(), nuke, path);

  os << templ.Head().Compile();
  
  unsigned index = 0;
  long long totalKBytes = 0;
  long long totalCredits = 0;
  int totalFiles = 0;
  auto& body = templ.Body();

  for (const auto& nukee : nuke.Nukees())
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

  auto& foot = templ.Foot();
  RegisterHeadFoot(foot, nuke, path);
  foot.RegisterValue("total_nukees", nuke.Nukees().size());
  foot.RegisterSize("total_size", totalKBytes);
  foot.RegisterSize("total_credits", totalCredits);
  foot.RegisterValue("total_files", totalFiles);
  os << foot.Compile();
  return os.str();
}

void Log(const std::string& command, const fs::RealPath& path, const db::nuking::Nuke& nuke)
{
  std::vector<std::pair<std::string, std::string>> pairs =
  {
    { "path",       path.ToString()                       },
    { "nuker",      acl::UIDToName(nuke.NukerUID())       },
    { "multiplier", std::to_string(nuke.Multiplier()) + 
                    (nuke.IsPercent() ? "%" : "")         },
    { "reason",     nuke.Reason()                         }
  };
  
  int count = 0;
  for (const auto& nukee : nuke.Nukees())
  {
    pairs.emplace_back("nukee" + std::to_string(++count), 
                       acl::UIDToName(nukee.UID()) + " " + std::to_string(nukee.Credits()));
  }
  logs::Event(command, pairs);
}

db::nuking::Nuke Nuke(const fs::VirtualPath& path, int multiplier, bool isPercent, 
                      const std::string& reason, acl::UserID nukerUID)
{
  class DoNuke
  {
    struct Nukee
    {
      
      long long kBytes;
      int files;
      long long credits;
      
      Nukee() : kBytes(0), files(0), credits(0) { }
    };
    
    const cfg::Config& config;
    fs::VirtualPath path;
    fs::RealPath real;
    int multiplier;
    bool isPercent;
    std::string reason;
    acl::UserID nukerUID;
    time_t modTime;
    long long totalKBytes;
    std::map<acl::UserID, Nukee> nukees;
    boost::optional<const cfg::Section&> section;
    boost::optional<db::nuking::Nuke> nuke;
    
    void CalculateNukees()
    {
      using namespace util::path;
      modTime = Status(real.ToString()).ModTime();
      
      for (const std::string& entry : RecursiveDirContainer(real.ToString()))
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
    
    void TakeCreditsEmpty(const std::string& sectionName)
    {
      if (!nukees.empty())
      {
        for (auto& kv : nukees)
        {
          kv.second.credits = config.NukeStyle().EmptyPenalty();
          auto user = acl::User::Load(kv.first);
          if (!user)
          {
            logs::Error("Unable to update user with uid %1% after nuke of: %2%", 
                        kv.first, real);
          }
          else
          {
            user->DecrSectionCreditsForce(sectionName, kv.second.credits);
          }
        }
      }
      else // otherwise we penalise the directory owner
      {
        auto owner = fs::GetOwner(real);
        auto& nukee = nukees[owner.UID()];
        nukee.credits = config.NukeStyle().EmptyPenalty();
        auto user = acl::User::Load(owner.UID());
        if (!user)
        {
          logs::Error("Unable to update user with uid %1% after nuke of: %2%", 
                      owner.UID(), real);
        }
        else
        {
          user->DecrSectionCreditsForce(sectionName, nukee.credits);
        }
      }
    }
    
    void TakeCreditsNotEmpty(const std::string& sectionName)
    {
      double percent = multiplier / 100.0;
      for (auto& kv : nukees)
      {
        auto user = acl::User::Load(kv.first);
        if (isPercent)
        {
          kv.second.credits = user->SectionCredits(sectionName) * percent;
          if (kv.second.credits < 0) kv.second.credits = 0;
        }
        else
        {
          int ratio = stats::UploadRatio(*user, path / "dummy-file", section);
          if (ratio == 0)
            kv.second.credits = 0;
          else
            kv.second.credits = kv.second.kBytes * ratio +
                                kv.second.kBytes * (multiplier - 1);
        }

        if (!user)
        {
          logs::Error("Unable to update user with uid %1% after nuke of: %2%", 
                      kv.first, real);
        }
        else
        {
          user->DecrSectionCreditsForce(sectionName, kv.second.credits);
        }
      }
    }
    
    void TakeCredits()
    {
      if (multiplier == 0) return;
      std::string sectionName(section && section->SeparateCredits() ? section->Name() : "");
      if (totalKBytes < config.NukeStyle().EmptyKBytes())
      {
        TakeCreditsEmpty(sectionName);
      }
      else
      {
        TakeCreditsNotEmpty(sectionName);
      }
    }
    
    void TakeStats()
    {
      std::string sectionName(section ? section->Name() : "");
      for (const auto& kv : nukees)
      {
        if (kv.second.kBytes > 0)
        {
          db::stats::UploadDecr(kv.first, kv.second.kBytes, modTime, 
                                sectionName, kv.second.files);
        }
      }
    }
    
    boost::optional<db::nuking::Nuke> LookupUnnuke()
    {
      boost::optional<db::nuking::Nuke> nuke;
      std::string id = GetNukeID(real);
      if (!id.empty()) nuke = db::nuking::LookupUnnukeByID(id);   
      if (!nuke) nuke = db::nuking::LookupUnnukeByPath(path.ToString());
      return nuke;
    }
    
    void UpdateDatabase()
    {
      std::vector<db::nuking::Nukee> nukees2;
      for (const auto& kv : nukees)
      {
        nukees2.emplace_back(kv.first, kv.second.kBytes, 
                             kv.second.files, kv.second.credits);
      }
      
      // remove old unnuke data if dir was unnuked in past
      auto unnuke = LookupUnnuke();
      if (unnuke) db::nuking::DelUnnuke(*unnuke);
      
      nuke = db::nuking::Nuke(path.ToString(), section ? section->Name() : "", 
                              reason, nukerUID, multiplier, isPercent, modTime, nukees2);
      db::nuking::AddNuke(*nuke);
    }

    void DeleteFiles()
    {
      for (const std::string& entry : util::path::RecursiveDirContainer(real.ToString()))
      {
        if (util::path::IsRegularFile(entry))
        {
          auto e = fs::DeleteFile(fs::RealPath(entry));
          if (!e) logs::Error("Unable to delete nuked file: %1%: %2%", entry, e.Message());
        }
      }
    }
    
    void DeleteDirectories()
    {
      for (const std::string& entry : util::path::RecursiveDirContainer(real.ToString()))
      {
        if (util::path::IsDirectory(entry))
        {
          auto e = fs::RemoveDirectory(fs::RealPath(entry));
          if (!e) logs::Error("Unable to remove nuked directory: %1%: %2%", entry, e.Message());
        }
      }
    }
    
    void DeleteContents()
    {
      try
      {
        DeleteFiles();
        DeleteDirectories();
      }
      catch (const util::SystemError& e)
      {
        logs::Error("Unable to delete nuked directory contents: %1%: %2%", real, e.Message());
      }
    }
    
    void Delete()
    {
      auto e = fs::RemoveDirectory(real);
      if (!e)
      {
        logs::Error("Unable to remove nuked directory: %1%: %2%", real, e.Message());
      }
    }
    
    void Rename()
    {
      assert(nuke);
      auto nukedPath = NukedPath(real);
      auto e = fs::Rename(real, nukedPath);
      if (!e)
      {
        logs::Error("Unable to rename nuked directory: %1% -> %2%: %3%", 
                    real, nukedPath, e.Message());
        SetNukeID(real, nuke->ID());
      }
      else
      {
        SetNukeID(nukedPath, nuke->ID());
      }
    }

    void ActionTheDirectory()
    {
      assert(nuke);
      auto action = config.NukeStyle().GetAction();
      if (action != cfg::NukeStyle::Keep)
      {
        DeleteContents();
      }

      if (action == cfg::NukeStyle::DeleteAll)
      {
        Delete();
      }
      else
      {
        Rename();
      }
    }

  public:
    DoNuke(const fs::VirtualPath& path, int multiplier, bool isPercent, 
           const std::string& reason, acl::UserID nukerUID) :
      config(cfg::Get()),
      path(path),
      real(fs::MakeReal(path)),
      multiplier(multiplier),
      isPercent(isPercent),
      reason(reason),
      nukerUID(nukerUID),
      modTime(0),
      totalKBytes(0),
      section(config.SectionMatch(path.ToString(), true))
    { }
  
    db::nuking::Nuke operator()()
    {
      CalculateNukees();
      TakeCredits();
      TakeStats();
      UpdateDatabase();
      ActionTheDirectory();
      Log("NUKE", real, *nuke);
      return *nuke;
    }
    
  } doNuke(path, multiplier, isPercent, reason, nukerUID);
  
  return doNuke();
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
    if (!cfg::Get().NukeMax().IsOkay(multiplier, isPercent))
    {
      control.Format(ftp::ActionNotOkay, "Invalid nuke multiplier / percent.");
      return;
    }    
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
  
  if (!acl::path::DirAllowed<acl::path::Nuke>(client.User(), path))
  {
    throw cmd::PermissionError();
  }

  try
  {
    auto nuke = Nuke(path, multiplier, isPercent, reason, client.User().ID());
    try
    {
      auto templ = text::Factory::GetTemplate("nuke");
      control.Format(ftp::CommandOkay, NukeTemplateCompile(nuke, templ, path));
    }
    catch (const text::TemplateError& e)
    {
      control.Format(ftp::ActionNotOkay, e.Message());
    }
  }
  catch (const util::RuntimeError& e)
  {
    control.Format(ftp::ActionNotOkay, "Error while nuking: %1%: %2%", args["path"], e.what());
  }
}

db::nuking::Nuke Unnuke(const fs::VirtualPath& path, const std::string& reason, 
                        acl::UserID nukerUID)
{
  class DoUnnuke
  {
    const cfg::Config& config;
    fs::VirtualPath path;
    fs::RealPath real;
    fs::RealPath nukedPath;
    std::string reason;
    acl::UserID nukerUID;
    db::nuking::Nuke nuke;
    boost::optional<const cfg::Section&> section;

    db::nuking::Nuke LookupNuke()
    {
      boost::optional<db::nuking::Nuke> nuke;
      std::string id = GetNukeID(nukedPath);
      if (!id.empty()) nuke = db::nuking::LookupNukeByID(id);   
      if (!nuke)
      {
        nuke = db::nuking::LookupNukeByPath(path.ToString());
        if (!nuke) throw util::RuntimeError("Unable to locate nuke data");
      }
      return *nuke;      
    }
    
    bool SeparateCredits()
    {
      auto it = config.Sections().find(nuke.Section());
      return it != config.Sections().end() && it->second.SeparateCredits();
    }

    void RestoreCredits()
    {
      std::string sectionName(SeparateCredits() ? nuke.Section() : "");
      for (const auto& nukee : nuke.Nukees())
      {
        auto user = acl::User::Load(nukee.UID());
        if (!user)
        {
          logs::Error("Unable to update user with uid %1% after unnuke of: %2%", 
                      nukee.UID(), real);
        }
        else
        {
          user->IncrSectionCredits(sectionName, nukee.Credits());
        }
      }
    }
    
    void RestoreStats()
    {
      for (const auto& nukee : nuke.Nukees())
      {
        if (nukee.KBytes() > 0 || nukee.Files() > 0)
        {
          db::stats::UploadIncr(nukee.UID(), nukee.KBytes(), nuke.ModTime(), 
                                nuke.Section(), nuke.Files());
        }
      }
    }

    void Rename()
    {
      RemoveNukeID(nukedPath);
      
      auto e = fs::Rename(nukedPath, real);
      if (!e && (!e.ValidErrno() || e.Errno() != ENOENT))
      {
        logs::Error("Unable to rename nuked directory: %1% -> %2%: %3%", 
                    nukedPath, real, util::Error::Failure(errno).Message());
      }
    }
    
    void UpdateDatabase()
    {
      db::nuking::DelNuke(nuke);
      nuke.Unnuke(reason, nukerUID);
      db::nuking::AddUnnuke(nuke);
      SetNukeID(real, nuke.ID());
    }
    
  public:
    DoUnnuke(const fs::VirtualPath& path, const std::string& reason, acl::UserID nukerUID) :
      config(cfg::Get()),
      path(path),
      real(fs::MakeReal(path)),
      nukedPath(NukedPath(real)),
      reason(reason),
      nukerUID(nukerUID),
      nuke(LookupNuke())
    { }

    db::nuking::Nuke operator()()
    {
      RestoreCredits();
      RestoreStats();
      Rename();
      UpdateDatabase();
      Log("UNNUKE", real, nuke);
      return nuke;
    }
    
  } doUnnuke(path, reason, nukerUID);
  
  return doUnnuke();
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

  try
  {
    auto nuke = Unnuke(path, reason, client.User().ID());
    try
    {
      auto templ = text::Factory::GetTemplate("unnuke");
        
      std::ostringstream os;
      RegisterHeadFoot(templ.Head(), nuke, path);
      RegisterHeadFoot(templ.Foot(), nuke, path);
      
      os << templ.Head().Compile();

      auto& body = templ.Body();
      unsigned index = 0;
      long long totalKBytes = 0;
      long long totalCredits = 0;
      int totalFiles = 0;
      for (const auto& nukee : nuke.Nukees())
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

      auto& foot = templ.Foot();
      foot.RegisterValue("total_nukees", nuke.Nukees().size());
      foot.RegisterSize("total_size", totalKBytes);
      foot.RegisterSize("total_credits", totalCredits);
      foot.RegisterValue("total_files", totalFiles);
      os << foot.Compile();
      
      control.Format(ftp::CommandOkay, os.str());
    }
    catch (const text::TemplateError& e)
    {
      control.Format(ftp::ActionNotOkay, e.Message());
    }
  }
  catch (const util::RuntimeError& e)
  {
    control.Format(ftp::ActionNotOkay, "Error while unnuking: %1%: %2%", args["path"], e.what());
  }
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
                           
  try
  {
    auto templ = text::Factory::GetTemplate(isUnnukes ? "unnukes" : "nukes");

    std::ostringstream os;
    os << templ.Head().Compile();

    auto& body = templ.Body();

    auto now = boost::posix_time::second_clock::local_time();
    unsigned index = 0;
    
    for (const auto& nuke : nukes)
    {
      using namespace boost::posix_time;
      body.RegisterValue("index", ++index);
      body.RegisterValue("datetime", to_simple_string(nuke.DateTime()));
      body.RegisterValue("modtime", to_simple_string(from_time_t(nuke.ModTime())));
      body.RegisterValue("age", Age(now - nuke.DateTime()));
      body.RegisterValue("files", nuke.Files());
      body.RegisterSize("size", nuke.KBytes());
      body.RegisterValue("directory", fs::Path(nuke.Path()).Basename().ToString());
      body.RegisterValue("path", nuke.Path());
      body.RegisterValue("section", nuke.Section());
      body.RegisterValue("nukees", FormatNukees(nuke.Nukees()));
      body.RegisterValue("reason", nuke.Reason());
      body.RegisterValue("nuker", acl::UIDToName(nuke.NukerUID()));
      
      std::string multiplier(std::to_string(nuke.Multiplier()));
      if (nuke.IsPercent()) multiplier += '%';    
      else multiplier += 'x';
      
      body.RegisterValue("multi", multiplier);

      os << body.Compile();
    }

    auto& foot = templ.Foot();
    foot.RegisterValue("count", nukes.size());
    os << foot.Compile();
    
    control.Format(ftp::CommandOkay, os.str());
  }
  catch (const text::TemplateError& e)
  {
    control.Format(ftp::ActionNotOkay, e.Message());
  }
}

} /* site namespace */
} /* cmd namespace */
