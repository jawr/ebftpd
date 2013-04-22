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

#include <cassert>
#include "ftp/logincounter.hpp"
#include "cfg/get.hpp"
#include "ftp/counter.hpp"

namespace ftp
{

CounterResult LoginCounter::Start(acl::UserID uid, int limit, bool kickLogin, bool exempt)
{
  std::lock_guard<std::mutex> lock(mutex);
  int& count = personal[uid];
  if (limit != -1 && count - kickLogin >= limit)
  {
    return CounterResult::PersonalFail;
  }
  int maxUsers = cfg::Config::MaxOnline().Users();
  if (exempt) maxUsers += cfg::Config::MaxOnline().ExemptUsers();
  if (global - kickLogin > maxUsers)
  {
//    return CounterResult::GlobalFail;
  }
  ++count;
  ++global;
  return CounterResult::Okay;
}

bool LoginCounter::LoginsUsed(acl::UserID uid, int limit)
{
  std::lock_guard<std::mutex> lock(mutex);
  return limit != -1 && personal[uid] >= limit;
}

void LoginCounter::Stop(acl::UserID uid)
{
  std::lock_guard<std::mutex> lock(mutex);
  int& count = personal[uid];
  assert(count > 0);
  --count;
  assert(global > 0);
  --global;
}

int LoginCounter::GlobalCount() const
{
  std::lock_guard<std::mutex> lock(mutex);
  return global;
}

} /* ftp namespace */

