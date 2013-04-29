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

#include <sstream>
#include "ftp/task/task.hpp"
#include "ftp/server.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "main.hpp"
#include "text/factory.hpp"
#include "text/error.hpp"
#include "cfg/error.hpp"
#include "ftp/data.hpp"
#include "ftp/client.hpp"

namespace ftp { namespace task
{

void Task::Push()
{
  ftp::Server::Get().PushTask(shared_from_this());
}

void KickUser::Execute(Server& server)
{
  int kicked = 0;
  for (auto& client : server.clients)
  {
    if (client.User().ID() == uid)
    {
      client.Interrupt();
      ++kicked;
      if (oneOnly) break;
    }
  }
  
  promise.set_value(kicked);
}

void LoginKickUser::Execute(Server& server)
{
  Result result;
  Client* toKick;  
  
  do
  {
    toKick = nullptr;
    result.logins = 0;
    result.idleTime = boost::posix_time::time_duration(0, 0, 0, 0);
    
    for (auto& client : server.clients)
    {
      if (client.State() == ftp::ClientState::LoggedIn && client.User().ID() == uid)
      {
        if (client.IdleTime() >= result.idleTime)
        {
          result.idleTime = client.IdleTime();
          toKick = &client;
        }
        
        ++result.logins;
      }
    }
  }
  while (toKick && toKick->State() != ftp::ClientState::LoggedIn);
  
  if (toKick)
  {
    toKick->Interrupt();
    result.kicked = true;
  }
  
  promise.set_value(result);
}

void ReloadConfig::Execute(Server&)
{
  Result configResult = Result::Okay;
  try
  {
    cfg::UpdateShared(cfg::Config::Load());
  }
  catch (const cfg::ConfigError& e)
  {
    logs::Error("Failed to load config: %1%", e.Message());
    configResult = Result::Fail;
  }
  
  Result templatesResult = Result::Okay;
  
  try
  {
    text::Factory::Initialize();
  }
  catch (const text::TemplateError& e)
  {
    logs::Error("Templates failed to initialise: %1%", e.Message());
    templatesResult = Result::Fail;
  }

  try
  {
    cfg::StopStartCheck();
  }
  catch (const cfg::StopStartNeeded& e)
  {
    logs::Error(e.Message());
    configResult = Result::StopStart;
  }
  promise.set_value(std::make_pair(configResult, templatesResult));
}

void Exit::Execute(Server&)
{
  logs::Debug("Server interrupted!");
  ftp::Server::Get().Shutdown();
}

void UserUpdate::Execute(Server& server)
{
  for (auto& client: server.clients)
  {
    if (client.State() == ClientState::LoggedIn && client.User().ID() == uid)
    {
      client.SetUserUpdated();
    }
  }
}

void ClientFinished::Execute(Server& server)
{
  server.CleanupClient(client);
}

}
}
