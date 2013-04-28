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

#ifndef __STATS_UTIL_HPP
#define __STATS_UTIL_HPP

#include <boost/optional/optional_fwd.hpp>
#include <cmath>

namespace boost { namespace posix_time
{
class ptime;
class time_duration;
}
}

namespace cfg
{
class  Section;
}

namespace acl
{
class User;
}

namespace fs
{
class VirtualPath;
}

namespace cfg
{
class Section;
}

namespace stats
{

double CalculateSpeed(long long bytes, const boost::posix_time::time_duration& duration);
double CalculateSpeed(long long bytes, const boost::posix_time::ptime& start, 
        const boost::posix_time::ptime& end);

boost::posix_time::time_duration SpeedLimitSleep(
      const boost::posix_time::time_duration& xfertime, 
      long long bytes, long long limitBytes);
        
std::string AutoUnitSpeedString(double speed);
std::string AutoUnitString(double kBytes);
std::string HighResSecondsString(const boost::posix_time::time_duration& duration);
std::string HighResSecondsString(const boost::posix_time::ptime& start, 
        const boost::posix_time::ptime& end);
int UploadRatio(const acl::User& user, const fs::VirtualPath& path, 
    const boost::optional<const cfg::Section&>& section);
int DownloadRatio(const acl::User& user, const fs::VirtualPath& path, 
    const boost::optional<const cfg::Section&>& section);
    
} /* stats namespace */

#endif
