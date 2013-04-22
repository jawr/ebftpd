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

#include <cstring>
#include <sstream>
#include <fstream>
#include "ftp/online.hpp"
#include "ftp/client.hpp"
#include "acl/user.hpp"
#include "util/error.hpp"
#include "main.hpp"

using namespace boost::interprocess;

namespace ftp
{

namespace
{
long ThreadIdToLong(const boost::thread::id& tid)
{
  std::stringstream ss;
  ss << tid;
  long ltid;
  ss >> std::hex >> ltid;
  return ltid;
}
}

std::unique_ptr<OnlineWriter> OnlineWriter::instance;
boost::posix_time::milliseconds OnlineTransferUpdater::interval(10);

OnlineXfer::OnlineXfer(stats::Direction direction, const boost::posix_time::ptime& start) :
  start(start),
  bytes(0),
  direction(direction)
{
}

OnlineClient::OnlineClient(
      acl::UserID uid, const std::string& ident, 
      const std::string& ip, const std::string& hostname,
      const std::string& workDir) :
  uid(uid), 
  lastCommand(boost::posix_time::second_clock::local_time()),
  command{0}
{
  strncpy(this->ident, ident.c_str(), sizeof(this->ident));
  strncpy(this->ip, ip.c_str(), sizeof(this->ip));
  strncpy(this->hostname, hostname.c_str(), sizeof(this->hostname));
  strncpy(this->workDir, workDir.c_str(), sizeof(this->workDir));
}

OnlineData::OnlineData(boost::interprocess::managed_shared_memory& segment, int maxClients) :
  clients(maxClients, std::hash<long>(), std::equal_to<long>(), segment.get_allocator<ShmOnlineMap::mapped_type>())
{
}

OnlineWriter::OnlineWriter(const std::string& id, int maxClients) :
  id(id), data(nullptr)
{
  OpenSharedMemory(maxClients);
}

void OnlineWriter::OpenSharedMemory(int maxClients)
{  
  try
  {
    segment.reset(new managed_shared_memory(open_or_create, id.c_str(), clientSize * (maxClients + 1)));

    segment->construct<OnlineData>("online")(*segment, maxClients);
    data = segment->find<OnlineData>("online").first;
    if (!data) throw util::SystemError(ENOMEM);
    scoped_lock<interprocess_mutex> lock(data->mutex);
    data->clients.clear();
  }
  catch (const interprocess_exception& e)
  {
    throw util::SystemError(ENOMEM);
  }
}

OnlineWriter::~OnlineWriter()
{
  segment = nullptr;
  shared_memory_object::remove(id.c_str());
}

void OnlineWriter::LoggedIn(long tid, Client& client, const std::string& workDir)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  data->clients.insert(std::make_pair(tid, OnlineClient(client.User().ID(), client.Ident(), 
              client.IP(), client.Hostname(), workDir)));  
}

void OnlineWriter::LoggedIn(const boost::thread::id& tid, Client& client, const std::string& workDir)
{
  LoggedIn(ThreadIdToLong(tid), client, workDir);
}

void OnlineWriter::LoggedOut(long tid)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  data->clients.erase(tid);
}

void OnlineWriter::LoggedOut(const boost::thread::id& tid)
{
  LoggedOut(ThreadIdToLong(tid));
}

void OnlineWriter::Command(long tid, const std::string& command)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  auto it = data->clients.find(tid);
  verify(it != data->clients.end());
  strncpy(it->second.command, command.c_str(), sizeof(it->second.command));
}

void OnlineWriter::Command(const boost::thread::id& tid, const std::string& command)
{
  Command(ThreadIdToLong(tid), command);
}

void OnlineWriter::Idle(long tid)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  auto it = data->clients.find(tid);
  verify(it != data->clients.end());
  it->second.command[0] = '\0';
  it->second.lastCommand = boost::posix_time::second_clock::local_time();
}

void OnlineWriter::Idle(const boost::thread::id& tid)
{
  Idle(ThreadIdToLong(tid));
}

void OnlineWriter::StartTransfer(long tid, stats::Direction direction, 
                                 const boost::posix_time::ptime& start)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  auto it = data->clients.find(tid);
  verify(it != data->clients.end());
  it->second.xfer.reset(OnlineXfer(direction, start));
}

void OnlineWriter::TransferUpdate(long tid, long long bytes)
{
  scoped_lock<interprocess_mutex> lock(data->mutex);
  auto it = data->clients.find(tid);
  verify(it != data->clients.end());
  assert(it->second.xfer);
  it->second.xfer->bytes = bytes;
}

void OnlineWriter::StopTransfer(long tid)
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
  segment = nullptr;
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

OnlineTransferUpdater::OnlineTransferUpdater(
        const boost::thread::id& tid, stats::Direction direction,
        const boost::posix_time::ptime& start) :
  tid(ThreadIdToLong(tid)),
  nextUpdate(start)
{
  OnlineWriter::Get().StartTransfer(this->tid, direction, start);
}

OnlineTransferUpdater::~OnlineTransferUpdater()
{
  OnlineWriter::Get().StopTransfer(tid);
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

