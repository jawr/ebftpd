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

#ifndef __TRANSFERCOUNTER_HPP
#define __TRANSFERCOUNTER_HPP

#include <mutex>
#include <unordered_map>
#include <functional>
#include "acl/types.hpp"

namespace ftp
{

enum class CounterResult : int;

class TransferCounter
{
  std::mutex mutex;
  int global;
  std::unordered_map<acl::UserID, int> personal;
  std::function<int(void)> getMaxGlobal;

  TransferCounter(const std::function<int(void)>& getMaxGlobal) :
    global(0), getMaxGlobal(getMaxGlobal)
  { }
  
  TransferCounter& operator=(const TransferCounter&) = delete;
  TransferCounter& operator=(TransferCounter&&) = delete;
  TransferCounter(const TransferCounter&) = delete;
  TransferCounter(TransferCounter&&) = delete;
  
public:
  CounterResult Start(acl::UserID uid, int limit, bool exempt);
  void Stop(acl::UserID uid);
  
  friend class Counter;
};

} /* ftp namespace */

#endif
