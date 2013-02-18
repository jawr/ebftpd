#include <sstream>
#include <iomanip>
#include "stats/util.hpp"
#include "ftp/client.hpp"
#include "acl/user.hpp"
#include "cfg/section.hpp"
#include "acl/misc.hpp"
#include "cfg/setting.hpp"
#include "acl/path.hpp"

namespace stats
{

std::string AutoUnitSpeedString(double speed)
{  
  return AutoUnitString(speed) + "/s";
}

std::string AutoUnitString(double kBytes)
{
  std::stringstream os;
  os << std::fixed << std::setprecision(2);
  if (kBytes < 1024.0) os << kBytes << "KB";
  else if (kBytes < 1024.0 * 1024.0) os << kBytes / 1024.0 << "MB";
  else os << (kBytes / (1024.0 * 1024.0)) << "GB";
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
    int ratio = client.User().SectionRatio(section->Name());
    if (ratio >= 0) return ratio;
  }
  
  auto cc = acl::CreditCheck(client.User(), path);
  if (cc && cc->Ratio() >= 0) return cc->Ratio();
  
  if (section &&  section->Ratio() >= 0) return section->Ratio();
  
  assert(client.User().DefaultRatio() >= 0);
  return client.User().DefaultRatio();
}

int DownloadRatio(const ftp::Client& client, const fs::VirtualPath& path, 
    const boost::optional<const cfg::Section&>& section)
{
  if (acl::path::FileAllowed<acl::path::Freefile>(client.User(), path)) return 0;
  
  if (section)
  {
    int ratio = client.User().SectionRatio(section->Name());
    if (ratio >= 0) return ratio;
  }
  
  auto cc = acl::CreditLoss(client.User(), path);
  if (cc && cc->Ratio() >= 0) return cc->Ratio();
  
  if (section &&  section->Ratio() >= 0) return section->Ratio();
  
  assert(client.User().DefaultRatio() >= 0);
  return client.User().DefaultRatio();
}

} /* stats namespace */
