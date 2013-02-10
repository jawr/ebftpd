#ifndef __DB_STATS_STAT_HPP
#define __DB_STATS_STAT_HPP

#include <ctime>
#include <string>
#include <vector>
#include "acl/user.hpp"

namespace stats
{
class Stat;
enum class Timeframe : unsigned;
enum class Direction : unsigned;
enum class SortField : unsigned;
}

namespace db { namespace stats
{

void Upload(const acl::User& user, long long kBytes, 
      long long xfertime, const std::string& section = "");

void Download(const acl::User& user, long long kBytes, 
      long long xfertime, const std::string& section = "");

void UploadDecr(const acl::User& user, long long kBytes, 
      time_t modTime, const std::string& section = "");

std::vector< ::stats::Stat> CalculateUserRanks(
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction, 
      ::stats::SortField sortField);

std::vector< ::stats::Stat> CalculateGroupRanks(
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction, 
      ::stats::SortField sortField);

::stats::Stat CalculateSingleUser(
      acl::UserID uid, 
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction);

::stats::Stat CalculateSingleGroup(
      acl::GroupID gid, 
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction);
}
}

#endif
