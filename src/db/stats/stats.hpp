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

#ifndef __DB_STATS_STAT_HPP
#define __DB_STATS_STAT_HPP

#include <ctime>
#include <string>
#include <vector>
#include "acl/types.hpp"

namespace stats
{
class Stat;
enum class Timeframe : unsigned;
enum class Direction : unsigned;
enum class SortField : unsigned;
}

namespace acl
{
class User;
}

namespace db { namespace stats
{

void Upload(const acl::User&    user, 
            long long           kBytes, 
            long long           xfertime, 
            const std::string&  section = "");

void Download(const acl::User&    user, 
              long long           kBytes, 
              long long           xfertime, 
              const std::string&  section = "");

void UploadIncr(acl::UserID         uid, 
                long long           kBytes,
                time_t              modTime, 
                const std::string&  section = "", 
                int                 files = 1);
      
void UploadDecr(acl::UserID         uid, 
                long long           kBytes, 
                time_t              modTime, 
                const std::string&  section = "", 
                int                 files = 1);

void UploadDecr(const acl::User&    user, 
                long long           kBytes, 
                time_t              modTime, 
                const std::string&  section = "", 
                int                 files = 1);

void Nuke(acl::UserID         uid, 
          long long           kBytes, 
          int                 files, 
          time_t              modTime, 
          const std::string&  section = "");
          
void Unnuke(acl::UserID         uid, 
            long long           kBytes, 
            int                 files, 
            time_t              modTime,
            const std::string&  section = "");
      
std::vector< ::stats::Stat> CalculateUserRanks(const std::string&   section, 
                                               ::stats::Timeframe   timeframe, 
                                               ::stats::Direction   direction, 
                                               ::stats::SortField   sortField);

std::vector< ::stats::Stat> CalculateGroupRanks(const std::string&  section, 
                                                ::stats::Timeframe  timeframe, 
                                                ::stats::Direction  direction, 
                                                ::stats::SortField  sortField);

::stats::Stat CalculateSingleUser(acl::UserID         uid, 
                                  const std::string&  section, 
                                  ::stats::Timeframe  timeframe, 
                                  ::stats::Direction  direction);

::stats::Stat CalculateSingleGroup(acl::GroupID         gid, 
                                   const std::string&   section, 
                                   ::stats::Timeframe   timeframe, 
                                   ::stats::Direction   direction);
} /* stats namespace */
} /* db namespace */

#endif
