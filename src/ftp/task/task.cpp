#include <sstream>
#include "ftp/task/task.hpp"
#include "ftp/listener.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "main.hpp"
#include "text/factory.hpp"
#include "text/error.hpp"
#include "cfg/error.hpp"

namespace ftp { namespace task
{

void Task::Push()
{
  ftp::Listener::PushTask(shared_from_this());
}

void KickUser::Execute(Listener& listener)
{
  unsigned kicked = 0;
  for (auto& client: listener.clients)
  {
    if (client.User().UID() == uid)
    {
      client.Interrupt();
      ++kicked;
    }
  }
  
  promise.set_value(kicked);
}

void LoginKickUser::Execute(Listener& listener)
{
  Result result;
  for (auto& client: listener.clients)
  {
    if (client.User().UID() == uid && client.State() == ftp::ClientState::LoggedIn)
    {
      if (!result.kicked)
      {
        client.Interrupt();
        result.kicked = true;
        result.idleTime = client.IdleTime();
      }
      
      ++result.logins;
    }
  }
  
  promise.set_value(result);
}

void GetOnlineUsers::Execute(Listener& listener)
{
  for (auto& client: listener.clients)
  {
    if (client.State() != ClientState::LoggedIn) continue;
    users.emplace_back(WhoUser(client.User(), client.Data().State(), client.IdleTime(), 
                       client.CurrentCommand(), client.Ident(), client.Hostname()));
  }
  
  promise.set_value(true);
}

void ReloadConfig::Execute(Listener&)
{
  Result configResult = Result::Okay;
  try
  {
    LoadConfig();
  }
  catch (const cfg::ConfigError& e)
  {
    logs::error << "Failed to load config: " + e.Message() << logs::endl;
    configResult = Result::Fail;
  }
  
  Result templatesResult = Result::Okay;
  
  try
  {
    text::Factory::Initalize();
  }
  catch (const text::TemplateError& e)
  {
    logs::error << "Templates failed to initalise: " << e.Message() << logs::endl;
    templatesResult = Result::Fail;
  }

  if (cfg::RequireStopStart()) configResult = Result::StopStart;
  promise.set_value(std::make_pair(configResult, templatesResult));
}

void Exit::Execute(Listener&)
{
  ftp::Listener::SetShutdown();
}

void UserUpdate::Execute(Listener& listener)
{
  for (auto& client: listener.clients)
  {
    if (client.User().UID() == uid) client.SetUserUpdated();
  }
}

}
}
