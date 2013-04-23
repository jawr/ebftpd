#ifndef __DB_NUKING_NUKING_HPP
#define __DB_NUKING_NUKING_HPP

#include <string>
#include <vector>
#include <ctime>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional_fwd.hpp>
#include "acl/types.hpp"

namespace db { namespace nuking
{

class Nukee
{
  acl::UserID uid;
  long long kBytes;
  int files;
  long long credits;

public:
  Nukee() : 
    uid(-1),
    kBytes(0),
    files(0),
    credits(0)
  { }
  
  Nukee(acl::UserID uid, long long kBytes, int files, long long credits) :
    uid(uid), 
    kBytes(kBytes), 
    files(files),
    credits(credits)
  { }
  
  acl::UserID UID() const { return uid; }
  long long KBytes() const { return kBytes; }
  int Files() const { return files; }
  long long Credits() const { return credits; }
};

class Nuke
{
  std::string id;
  std::string path;
  std::string section;
  int multiplier;
  bool isPercent;
  time_t modTime;
  boost::posix_time::ptime dateTime;
  std::vector<Nukee> nukees;

public:
  Nuke(const std::string& path, const std::string& section, 
       int multiplier, bool isPercent, time_t modTime,
       const std::vector<Nukee>& nukees) :
    path(path), 
    section(section),
    multiplier(multiplier),
    isPercent(isPercent),
    modTime(modTime),
    nukees(nukees)
  { }

  Nuke(const std::string& id, const std::string& path, 
       const std::string& section, int multiplier, 
       bool isPercent, time_t modTime,
       const boost::posix_time::ptime& dateTime, 
       const std::vector<Nukee>& nukees) :
    id(id),
    path(path), 
    section(section),
    multiplier(multiplier), 
    isPercent(isPercent),
    modTime(modTime),
    dateTime(dateTime),
    nukees(nukees)
  { }
  
  const std::string& ID() const { return id; }
  const std::string& Path() const { return path; }
  const std::string& Section() const { return section; }
  int Multiplier() const { return multiplier; }
  bool IsPercent() const { return isPercent; }
  time_t ModTime() const { return modTime; }
  const boost::posix_time::ptime& DateTime() const { return dateTime; }
  const std::vector<Nukee>& Nukees() const { return nukees; }
};

void AddNuke(const Nuke& nuke);
bool DelNuke(const Nuke& nuke);
boost::optional<Nuke> LookupNuke(const std::string& id);
void AddUnnuke(const Nuke& nuke);
bool DelUnnuke(const Nuke& nuke);
std::vector<Nuke> NewestNukes(int limit);
std::vector<Nuke> NewestUnnukes(int limit);

} /* nuking namespace */
} /* db namespace */

#endif
