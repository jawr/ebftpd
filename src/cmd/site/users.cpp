#include <boost/optional/optional.hpp>
#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/users.hpp"
#include "util/error.hpp"
#include "acl/types.hpp"
#include "acl/groupcache.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "db/stats/stat.hpp"
#include "logs/logs.hpp"
#include "stats/stat.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/error.hpp"

namespace cmd { namespace site
{

void USERSCommand::Execute()
{
  std::vector<acl::User> users;
  
  if (args.size() == 2)
    users = db::user::GetByACL(args[1]);
  else
    users = db::user::GetAll();

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
      ::stats::Stat upStat = db::stats::CalculateSingleUser(user.UID(), "", 
                            ::stats::Timeframe::Alltime, ::stats::Direction::Upload);
      ::stats::Stat dnStat = db::stats::CalculateSingleUser(user.UID(), "", 
                            ::stats::Timeframe::Alltime, ::stats::Direction::Download);

      body.Reset();
      body.RegisterValue("user", user.Name());
      body.RegisterValue("group", acl::GroupCache::GIDToName(user.PrimaryGID()));
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

// end
}
}
