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
#include <boost/thread/tss.hpp>
#include <mutex>
#include <boost/signals2.hpp>
#include "cfg/get.hpp"
#include "logs/logs.hpp"
#include "util/string.hpp"
#include "cfg/error.hpp"

namespace cfg
{

namespace
{

boost::thread_specific_ptr<Config> thisThread;
std::mutex sharedMutex;
std::shared_ptr<Config> shared;
boost::signals2::signal<void()> updated;

}

void UpdateShared(const std::shared_ptr<Config> newShared)
{
  {
    std::lock_guard<std::mutex> lock(sharedMutex);
    shared = newShared;
  }
  
  updated();
}

void UpdateLocal()
{
  Config* config = thisThread.get();
  std::lock_guard<std::mutex> lock(sharedMutex);
  if (config && shared->Version() <= config->Version()) return;
  thisThread.reset(new Config(*shared));
}

const Config& Get()
{
  assert(shared.get()); // program must never call Get until a valid config is loaded
  Config* config = thisThread.get();
  if (!config)
  {
    UpdateLocal();
    config = thisThread.get();
    assert(config);
  }
  return *config;
}

void StopStartCheck()
{
  const Config& old = cfg::Get();
  std::vector<std::string> settings;

  if (shared->ValidIp() != old.ValidIp()) settings.push_back("valid_ip");
  if (shared->Port() != old.Port()) settings.push_back("port");
  if (shared->TlsCertificate() != old.TlsCertificate()) settings.push_back("tls_certificate");
  if (shared->TlsCiphers() != old.TlsCiphers()) settings.push_back("tls_ciphers");
  
  if (shared->Database() != old.Database()) settings.push_back("db_*");
  if (shared->MaxUsers() != old.MaxUsers()) settings.push_back("max_users");
  
  if (!settings.empty())
  {
    throw StopStartNeeded("Full stop start required for these settings: " + 
                          util::Join(settings, ","));
  }
}

void ConnectUpdatedSlot(const std::function<void()>& slot)
{
  updated.connect(slot);
}

}
