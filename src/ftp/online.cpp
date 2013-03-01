#include <sstream>
#include <fstream>
#include "ftp/online.hpp"
#include "ftp/client.hpp"
#include "acl/user.hpp"
#include "main.hpp"

using namespace boost::interprocess;

namespace ftp
{

std::unique_ptr<OnlineWriter> OnlineWriter::instance;
boost::posix_time::milliseconds OnlineTransferUpdater::interval(10);

OnlineXfer::OnlineXfer(stats::Direction direction, const boost::posix_time::ptime& start) :
  start(start),
  bytes(0),
  direction(direction)
{
}

OnlineClientData::OnlineClientData(
      ShmCharAlloc& alloc, acl::UserID uid, const std::string& ident, 
      const std::string& ip, const std::string& hostname,
      const std::string& workDir) :
  uid(uid), 
  lastCommand(boost::posix_time::second_clock::local_time()),
  command("", alloc), 
  workDir(workDir.c_str(), alloc), 
  ident(ident.c_str(), alloc), 
  ip(ip.c_str(), alloc), 
  hostname(hostname.c_str(), alloc)
{
}

OnlineClient::OnlineClient(const OnlineClientData& data) :
  uid(data.uid), 
  lastCommand(data.lastCommand),
  command(data.command.c_str()), 
  workDir(data.workDir.c_str()),
  ident(data.ident.c_str()),
  ip(data.ip.c_str()), 
  hostname(data.hostname.c_str()),
  xfer(data.xfer)
{
}

OnlineData::OnlineData(boost::interprocess::managed_shared_memory& segment) :
  clients(std::less<boost::thread::id>(), 
          ShmOnlineMapAlloc(segment.get_segment_manager()))
{
}

OnlineWriter::OnlineWriter(const std::string& id) :
  id(id), data(nullptr)
{
  OpenSharedMemory();
}

void OnlineWriter::OpenSharedMemory()
{  
  segment.reset(new managed_shared_memory(open_or_create, id.c_str(), 65535));

  segment->construct<OnlineData>("online")(*segment);
  data = segment->find<OnlineData>("online").first;

  scoped_lock<interprocess_mutex> lock(data->mutex);
  data->clients.clear();
}

OnlineWriter::~OnlineWriter()
{
  shared_memory_object::remove(id.c_str());
}

void OnlineWriter::LoggedIn(
      const boost::thread::id& tid, acl::UserID uid, const std::string& ident, 
      const std::string& ip, const std::string& hostname,
      const std::string& workDir)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  ShmCharAlloc alloc(segment->get_segment_manager());
  data->clients.insert(std::make_pair(tid, OnlineClientData(alloc, uid, ident, ip, hostname, workDir)));  
}

void OnlineWriter::LoggedIn(const boost::thread::id& tid, Client& client, const std::string& workDir)
{
  LoggedIn(tid, client.User().ID(), client.Ident(), client.IP(), client.Hostname(), workDir);
}

void OnlineWriter::LoggedOut(const boost::thread::id& tid)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  data->clients.erase(tid);
}

void OnlineWriter::Command(const boost::thread::id& tid, const std::string& command)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  auto it = data->clients.find(tid);
  verify(it != data->clients.end());
  it->second.command.assign(command.c_str());
}

void OnlineWriter::Idle(const boost::thread::id& tid)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  auto it = data->clients.find(tid);
  verify(it != data->clients.end());
  it->second.command.clear();
  it->second.lastCommand = boost::posix_time::second_clock::local_time();
}

void OnlineWriter::StartTransfer(const boost::thread::id& tid, stats::Direction direction, const boost::posix_time::ptime& start)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  auto it = data->clients.find(tid);
  verify(it != data->clients.end());
  it->second.xfer.reset(OnlineXfer(direction, start));
}

void OnlineWriter::TransferUpdate(const boost::thread::id& tid, long long bytes)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  auto it = data->clients.find(tid);
  verify(it != data->clients.end());
  assert(it->second.xfer);
  it->second.xfer->bytes = bytes;
}

void OnlineWriter::StopTransfer(const boost::thread::id& tid)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  auto it = data->clients.find(tid);
  verify(it != data->clients.end());
  assert(it->second.xfer);
  it->second.xfer = boost::none;
}

OnlineReaderIterator::OnlineReaderIterator(const OnlineReader* const reader) :
  reader(reader)
{
  if (reader && reader->data) iter.reset(reader->data->clients.begin());
}

OnlineReaderIterator::OnlineReaderIterator(const OnlineReader* const reader, bool /* end */) :
  reader(reader)
{
  if (reader && reader->data) iter.reset(reader->data->clients.end());
}

OnlineReaderIterator& OnlineReaderIterator::operator++()
{
  client = boost::none;
  verify(iter && reader && reader->locked);
  ++(*iter);
  return *this;
}

OnlineReaderIterator OnlineReaderIterator::operator++(int)
{
  client = boost::none;
  verify(iter && reader && reader->locked);
  OnlineReaderIterator temp(*this);
  ++(*iter);
  return temp;
}

OnlineReader::OnlineReader(const std::string& id) :    
  data(nullptr), locked(false)
{
  try
  {
    segment.reset(new managed_shared_memory(open_only, id.c_str()));
    data = segment->find<OnlineData>("online").first;
  }
  catch (const boost::interprocess::interprocess_exception& e)
  {
    // server must not be loaded, reader is empty
  }
}

OnlineReader::~OnlineReader()
{
  shared_memory_object::remove(id.c_str());
}

OnlineReaderIterator OnlineReader::begin() const
{
  verify(locked);
  return OnlineReaderIterator(this);
}

OnlineReaderIterator OnlineReader::end() const
{
  verify(locked);
  return OnlineReaderIterator(this, true);
}

OnlineReader::size_type OnlineReader::size() const
{
  if (!data) return 0;
  if (locked) return data->clients.size();
  else
  {
    scoped_lock<interprocess_mutex> lock(data->mutex);
    return data->clients.size();
  }
}

void OnlineReader::Lock() const
{
  locked = true;
  if (data) data->mutex.lock();
}

void OnlineReader::Unlock() const
{
  locked = false;
  if (data) data->mutex.unlock();
}

std::string SharedMemoryID(pid_t pid)
{
  if (pid == -1) pid = getpid();
  std::ostringstream os;
  os << programName << "-" << pid;
  return os.str();
}

bool SharedMemoryID(const std::string& pidFile, std::string& id)
{
  std::ifstream f(pidFile.c_str());
  if (!f) return false;
  
  pid_t pid;
  if (!(f >> pid)) return false;

  id = SharedMemoryID(pid);
  return true;
}

} /* ftp namespace */

