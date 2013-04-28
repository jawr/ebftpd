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
  std::string reason;
  acl::UserID nukerUID;
  int multiplier;
  bool isPercent;
  time_t modTime;
  boost::posix_time::ptime dateTime;
  std::vector<Nukee> nukees;

public:
  Nuke(const std::string& path, const std::string& section, 
       const std::string& reason, acl::UserID nukerUID,
       int multiplier, bool isPercent, time_t modTime,
       const std::vector<Nukee>& nukees);

  Nuke(const std::string& id, const std::string& path, 
       const std::string& section, const std::string& reason, 
       acl::UserID nukerUID, int multiplier, 
       bool isPercent, time_t modTime,
       const boost::posix_time::ptime& dateTime, 
       const std::vector<Nukee>& nukees);
  
  const std::string& ID() const { return id; }
  const std::string& Path() const { return path; }
  const std::string& Section() const { return section; }
  const std::string& Reason() const { return reason; }
  acl::UserID NukerUID() const { return nukerUID; }
  int Multiplier() const { return multiplier; }
  bool IsPercent() const { return isPercent; }
  time_t ModTime() const { return modTime; }
  long long KBytes() const;
  int Files() const;
  const boost::posix_time::ptime& DateTime() const { return dateTime; }
  const std::vector<Nukee>& Nukees() const { return nukees; }
  
  void Unnuke(const std::string& reason, acl::UserID nukerUID);
};

void AddNuke(const Nuke& nuke);
bool DelNuke(const Nuke& nuke);
boost::optional<Nuke> LookupNukeByID(const std::string& id);
boost::optional<Nuke> LookupNukeByPath(const std::string& path);
void AddUnnuke(const Nuke& nuke);
bool DelUnnuke(const Nuke& nuke);
boost::optional<Nuke> LookupUnnukeByID(const std::string& id);
boost::optional<Nuke> LookupUnnukeByPath(const std::string& path);
std::vector<Nuke> NewestNukes(int limit);
std::vector<Nuke> NewestUnnukes(int limit);

} /* nuking namespace */
} /* db namespace */

#endif
