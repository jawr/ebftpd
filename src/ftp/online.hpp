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

#include <iterator>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/segment_manager.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <limits.h>
#if defined(__FreeBSD__)
#include <sys/param.h>
#endif
#include <netinet/in.h>
#include "acl/types.hpp"
#include "stats/types.hpp"
#include "util/verify.hpp"

namespace ftp
{

struct OnlineXfer
{
  boost::posix_time::ptime start;
  long long bytes;
  stats::Direction direction;
  
  OnlineXfer(stats::Direction direction, const boost::posix_time::ptime& start);
};

struct OnlineClient
{
  static const unsigned maximumIdentLength = 513;

  acl::UserID uid;
  boost::posix_time::ptime lastCommand;
  char command[BUFSIZ];
  char workDir[PATH_MAX];
  char ident[maximumIdentLength];
  char ip[INET6_ADDRSTRLEN];
#if defined(__FreeBSD__)
  char hostname[MAXHOSTNAMELEN];
#else
  char hostname[HOST_NAME_MAX];
#endif
  
  boost::optional<OnlineXfer> xfer;

  OnlineClient(acl::UserID uid, const std::string& ident, 
               const std::string& ip, const std::string& hostname,
               const std::string& workDir);
               
  bool IsIdle() const { return command[0] == '\0'; }
};

typedef boost::interprocess::managed_shared_memory::segment_manager SegmentManager;
typedef boost::interprocess::allocator<std::pair<const long, OnlineClient>, 
                                       SegmentManager> ShmOnlineMapAlloc;
typedef boost::unordered_map<long, OnlineClient, std::hash<long>, std::equal_to<long>, ShmOnlineMapAlloc> ShmOnlineMap;

struct OnlineData
{
  ShmOnlineMap clients;
  boost::interprocess::interprocess_mutex mutex;
  
  OnlineData(boost::interprocess::managed_shared_memory& segment, int maxClients);
};

class Client;
class OnlineTransferUpdater;

class OnlineWriter
{
  std::string id;
  std::unique_ptr<boost::interprocess::managed_shared_memory> segment;
  OnlineData* data;

  static std::unique_ptr<OnlineWriter> instance;
  constexpr static float clientOverhead = 0.02;
  constexpr static size_t clientSize = sizeof(OnlineClient) * (1 + clientOverhead);

  OnlineWriter(const std::string& id, int maxClients);
  void OpenSharedMemory(int maxClients);

  void LoggedIn(long tid, Client& client, const std::string& workDir);
  void LoggedOut(long tid);
  void Command(long tid, const std::string& command);
  void Idle(long tid);

  void StartTransfer(long tid, stats::Direction direction, const boost::posix_time::ptime& start);
  void TransferUpdate(long tid, long long bytes);
  void StopTransfer(long tid);
  
public:
  ~OnlineWriter();
  
  void LoggedIn(const boost::thread::id& tid, Client& client, const std::string& workDir);
  void LoggedOut(const boost::thread::id& tid);
  void Command(const boost::thread::id& tid, const std::string& command);
  void Idle(const boost::thread::id& tid);
  
  static void Initialise(const std::string& id, int maxClients)
  {
    instance.reset(new OnlineWriter(id, maxClients));
  }
  
  static void Cleanup()
  {
    instance = nullptr;
  }
  
  static OnlineWriter& Get() { return *instance; }
  
  friend class OnlineTransferUpdater;
};

class OnlineReaderLock;
class OnlineReader;

class OnlineReaderIterator : public std::iterator<std::forward_iterator_tag, OnlineClient>
{
  const OnlineReader* const reader;
  boost::optional<ShmOnlineMap::iterator> iter;
  mutable boost::optional<OnlineClient> client;
    
  OnlineReaderIterator(const OnlineReader* const reader, bool end); 
  OnlineReaderIterator(const OnlineReader* const reader);
  
public:
    
  OnlineReaderIterator& operator++();
  OnlineReaderIterator operator++(int);
  
  bool operator==(const OnlineReaderIterator& rhs) const
  {
    if (!iter && !rhs.iter) return true;
    verify(iter);
    return *iter == *rhs.iter;
  }
  
  bool operator!=(const OnlineReaderIterator& rhs) const
  {
    return !operator==(rhs);
  }
  
  const OnlineClient& operator*() const
  {
    if (!client) client.reset((*iter)->second);
    return *client;
  }

  const OnlineClient* operator->() const
  {
    if (!client) client.reset((*iter)->second);
    return &*client;
  }
  
  friend class OnlineReader;
};

class OnlineReader
{
  std::string id;
  std::unique_ptr<boost::interprocess::managed_shared_memory> segment;
  OnlineData* data;
  mutable bool locked;
  
  void Lock() const;
  void Unlock() const;
  
public:
  typedef OnlineReaderIterator const_iterator;
  typedef OnlineClient value_type;
  typedef ShmOnlineMap::size_type size_type;

  OnlineReader(const std::string& id);
  ~OnlineReader();
  
  OnlineReaderIterator begin() const;
  OnlineReaderIterator end() const;
  
  size_type size() const;

  friend class OnlineReaderLock;
  friend class OnlineReaderIterator;
};

class OnlineReaderLock
{
  const OnlineReader& reader;
  bool released;
  
public:
  OnlineReaderLock(const OnlineReader& reader) :
    reader(reader), released(false)
  {
    reader.Lock();
  }
  
  ~OnlineReaderLock()
  {
    if (!released)
    {
      reader.Unlock();
    }
  }
  
  void Release()
  {
    verify(!released);
    reader.Unlock();
    released = true;
  }
};

class OnlineTransferUpdater
{
  long tid;
  boost::posix_time::ptime nextUpdate;
  
  static boost::posix_time::milliseconds interval;
  
public:
  OnlineTransferUpdater(const boost::thread::id& tid, stats::Direction direction,
                        const boost::posix_time::ptime& start);
  
  ~OnlineTransferUpdater();
  
  void Update(long long bytes)
  {
    auto now = boost::posix_time::microsec_clock::local_time();
    if (now >= nextUpdate)
    {
      OnlineWriter::Get().TransferUpdate(tid, bytes);
      nextUpdate = now + interval;
    }
  }
};

std::string SharedMemoryID(pid_t pid = -1);
bool SharedMemoryID(const std::string& pidFile, std::string& id);

} /* ftp namespace */
