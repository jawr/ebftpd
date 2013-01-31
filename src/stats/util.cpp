#include <sstream>
#include <iomanip>
#include "stats/util.hpp"
#include "ftp/client.hpp"
#include "acl/userprofile.hpp"
#include "cfg/section.hpp"
#include "acl/credits.hpp"
#include "cfg/setting.hpp"
#include "acl/path.hpp"

namespace stats
{

std::string AutoUnitSpeedString(double speed)
{  
  return AutoUnitString(speed) + "/s";
}

std::string AutoUnitString(double amount)
{
  std::stringstream os;
  os << std::fixed << std::setprecision(2);
  if (amount < 1024.0) os << amount << "B";
  else if (amount < 1024.0 * 1024.0) os << (amount / 1024.0) << "KB";
  else if (amount < 1024.0 * 1024.0 * 1024.0) os << (amount / (1024.0  * 1024.0)) << "MB";
  else os << (amount / (1024.0 * 1024.0 * 1024.0)) << "GB";
  return os.str();
}

std::string HighResSecondsString(const boost::posix_time::time_duration& duration)
{
  std::stringstream os;
  
  if (duration.total_microseconds() < 100000)
  {
    os << std::fixed << std::setprecision(3);
    os << (duration.total_microseconds() / 1000.0 ) << "ms";
  }
  else
  {
    os << std::fixed << std::setprecision(2);
    os << (duration.total_microseconds() / 1000000.0) << "s";
  }
  
  return os.str();
}

int UploadRatio(const ftp::Client& client, const fs::VirtualPath& path, 
    const boost::optional<const cfg::Section&>& section)
{
  if (section)
  {
    int ratio = client.UserProfile().Ratio(section->Name());
    if (ratio >= 0) return ratio;
  }
  
  auto cc = acl::CreditCheck(client.User(), path);
  if (cc && cc->Ratio() >= 0) return cc->Ratio();
  
  if (section &&  section->Ratio() >= 0) return section->Ratio();
  
  assert(client.UserProfile().Ratio("") >= 0);
  return client.UserProfile().Ratio("");
}

int DownloadRatio(const ftp::Client& client, const fs::VirtualPath& path, 
    const boost::optional<const cfg::Section&>& section)
{
  if (acl::path::FileAllowed<acl::path::Freefile>(client.User(), path)) return 0;
  
  if (section)
  {
    int ratio = client.UserProfile().Ratio(section->Name());
    if (ratio >= 0) return ratio;
  }
  
  auto cc = acl::CreditLoss(client.User(), path);
  if (cc && cc->Ratio() >= 0) return cc->Ratio();
  
  if (section &&  section->Ratio() >= 0) return section->Ratio();
  
  assert(client.UserProfile().Ratio("") >= 0);
  return client.UserProfile().Ratio("");
}

} /* stats namespace */
