#include <iterator>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/segment_manager.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include "acl/types.hpp"
#include "stats/types.hpp"
#include "util/verify.hpp"

namespace ftp
{

typedef boost::interprocess::managed_shared_memory::segment_manager SegmentManager;
typedef boost::interprocess::allocator<char, SegmentManager> ShmCharAlloc;
typedef boost::interprocess::basic_string<char, std::char_traits<char>, ShmCharAlloc> ShmString;
typedef boost::interprocess::allocator<void, SegmentManager> ShmVoidAlloc;

struct OnlineXfer
{
	boost::posix_time::ptime start;
	long long bytes;
	stats::Direction direction;
  
  OnlineXfer(stats::Direction direction, const boost::posix_time::ptime& start);
};

struct OnlineClientData
{
	acl::UserID uid;
	boost::posix_time::ptime lastCommand;
	ShmString command;
	ShmString workDir;
	ShmString ident;
	ShmString ip;
	ShmString hostname;
	
  boost::optional<OnlineXfer> xfer;
	
	OnlineClientData(ShmCharAlloc& alloc, acl::UserID uid, const std::string& ident, 
                   const std::string& ip, const std::string& hostname,
                   const std::string& workDir);
};

struct OnlineClient
{
	acl::UserID uid;
	boost::posix_time::ptime lastCommand;
	std::string command;
	std::string workDir;
	std::string ident;
	std::string ip;
	std::string hostname;
	
  boost::optional<OnlineXfer> xfer;
	
	OnlineClient(const OnlineClientData& data);
};

typedef boost::interprocess::allocator<std::pair<const boost::thread::id, OnlineClientData>, SegmentManager> ShmOnlineMapAlloc;
typedef boost::interprocess::map<boost::thread::id, OnlineClientData, std::less<boost::thread::id>, ShmOnlineMapAlloc> ShmOnlineMap;

struct OnlineData
{
  ShmOnlineMap clients;
  boost::interprocess::interprocess_mutex mutex;
  
  OnlineData(boost::interprocess::managed_shared_memory& segment);
};

class Client;

class OnlineWriter
{
  std::string id;
  std::unique_ptr<boost::interprocess::managed_shared_memory> segment;
  OnlineData* data;

	static std::unique_ptr<OnlineWriter> instance;

  OnlineWriter(const std::string& id);
  void OpenSharedMemory();

public:
  ~OnlineWriter();
	void LoggedIn(const boost::thread::id& tid, acl::UserID uid, const std::string& ident, 
								const std::string& ip, const std::string& hostname,
								const std::string& workDir);
  void LoggedIn(const boost::thread::id& tid, Client& client, const std::string& workDir);
	void LoggedOut(const boost::thread::id& tid);
	void Command(const boost::thread::id& tid, const std::string& command);
	void Idle(const boost::thread::id& tid);
	void StartTransfer(const boost::thread::id& tid, stats::Direction direction, const boost::posix_time::ptime& start);
	void TransferUpdate(const boost::thread::id& tid, long long bytes);
	void StopTransfer(const boost::thread::id& tid);
  
	static void Initialise(const std::string& id)
  {
    instance.reset(new OnlineWriter(id));
  }
  
  static OnlineWriter& Get() { return *instance; }
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

std::string SharedMemoryID(pid_t pid = -1);
bool SharedMemoryID(const std::string& pidFile, std::string& id);

} /* ftp namespace */
