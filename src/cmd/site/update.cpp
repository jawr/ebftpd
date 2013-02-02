#include <sstream>
#include "cmd/site/update.hpp"
#include "util/string.hpp"
#include "fs/dircontainer.hpp"
#include "fs/path.hpp"
#include "fs/status.hpp"
#include "cfg/get.hpp"
#include "db/index/index.hpp"
#include "acl/path.hpp"

namespace cmd { namespace site
{

void UPDATECommand::Execute()
{
  std::string pathStr = args.size() == 2 ? argStr : ".";
  auto pathMask = fs::PathFromUser(pathStr);
  try
  {
    using util::string::WildcardMatch;
  
    unsigned addedCount = 0;
    for (const auto& entry : fs::DirContainer(client.User(), pathMask.Dirname()))
    {
      if (!WildcardMatch(pathMask.Basename().ToString(), entry.ToString())) continue;

      fs::VirtualPath entryPath(pathMask.Dirname() / entry);
      if (!acl::path::DirAllowed<acl::path::Indexed>(client.User(), entryPath)) continue;

      try
      {
        if (fs::Status(fs::MakeReal(entryPath)).IsDirectory())
        {
          auto section = cfg::Get().SectionMatch(entryPath);
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

} /* site namespace */
} /* cmd namespace */
