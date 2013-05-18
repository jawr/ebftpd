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

#ifndef __DB_REPLICATOR_HPP
#define __DB_REPLICATOR_HPP

#include <atomic>
#include <future>
#include <memory>
#include <boost/thread/thread.hpp>
#include <mutex>
#include "db/replicable.hpp"

namespace mongo
{
class BSONObj;
}

namespace db
{

class Replicator
{
  boost::thread thread;
  std::vector<std::shared_ptr<Replicable>> caches;

  static std::unique_ptr<Replicator> instance;
  static const int maximumRetries = 20;
  static const long retryInterval = 10;
  
  Replicator() = default;
  
  void Run();  
  void LogFailed(const std::list<std::shared_ptr<Replicable>>& failed);
  void Replicate(const mongo::BSONObj& entry);
  void Populate();
  
public:
  void Start();
  void Stop();
  
  bool Register(const std::shared_ptr<Replicable>& cache);

  static Replicator& Get()
  {
    if (!instance) instance.reset(new Replicator());
    return *instance;
  }  
};

} /* db namespace */

#endif
