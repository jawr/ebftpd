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
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "stats/util.hpp"
#include "ftp/client.hpp"
#include "acl/user.hpp"
#include "cfg/section.hpp"
#include "acl/misc.hpp"
#include "cfg/setting.hpp"
#include "acl/path.hpp"

namespace stats
{

double CalculateSpeed(long long bytes, const boost::posix_time::time_duration& duration)
{
  double seconds = duration.total_microseconds() / 1000000.0;
  return seconds == 0.0 ? bytes : bytes / seconds;
}

double CalculateSpeed(long long bytes, const boost::posix_time::ptime& start, 
        const boost::posix_time::ptime& end)
{
  return CalculateSpeed(bytes, end - start);
}

boost::posix_time::time_duration SpeedLimitSleep( 
      const boost::posix_time::time_duration& xfertime, 
      long long bytes, long long limitBytes)
{
  auto minXfertime = boost::posix_time::microseconds((bytes / static_cast<double>(limitBytes)) * 1000000);
  if (minXfertime < xfertime) return boost::posix_time::microseconds(0);
  return std::min<boost::posix_time::time_duration>(boost::posix_time::
          microseconds(100000), minXfertime - xfertime);
}


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

std::string HighResSecondsString(const boost::posix_time::ptime& start, 
        const boost::posix_time::ptime& end)
{
  return HighResSecondsString(end - start);
}

int UploadRatio(const acl::User& user, const fs::VirtualPath& path, 
    const boost::optional<const cfg::Section&>& section)
{
  if (section)
  {
    int ratio = user.SectionRatio(section->Name());
    if (ratio >= 0) return ratio;
  }
  
  auto cc = acl::CreditCheck(user, path);
  if (cc && cc->Ratio() >= 0) return cc->Ratio();
  
  if (section &&  section->Ratio() >= 0) return section->Ratio();
  
  assert(user.DefaultRatio() >= 0);
  return user.DefaultRatio();
}

int DownloadRatio(const acl::User& user, const fs::VirtualPath& path, 
    const boost::optional<const cfg::Section&>& section)
{
  if (acl::path::FileAllowed<acl::path::Freefile>(user, path)) return 0;
  
  if (section)
  {
    int ratio = user.SectionRatio(section->Name());
    if (ratio == 0) return 0;
  }
  
  auto cl = acl::CreditLoss(user, path);
  if (cl && cl->Ratio() >= 0) return cl->Ratio();
  
  if (section &&  section->Ratio() >= 0) return section->Ratio();
  
  assert(user.DefaultRatio() >= 0);
  return user.DefaultRatio() == 0 ? 0 : 1;
}

} /* stats namespace */
