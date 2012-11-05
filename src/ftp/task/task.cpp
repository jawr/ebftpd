#include <sstream>
#include "ftp/task/task.hpp"
#include "ftp/listener.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "main.hpp"

namespace ftp { namespace task
{

void KickUser::Execute(Listener& listener)
{
  for (auto& client: listener.clients)
  {
    if (client.User().UID() == uid) client.Interrupt();
  }
}

void GetOnlineUsers::Execute(Listener& listener)
{
  for (auto& client: listener.clients)
  {
    if (client.State() != ClientState::LoggedIn) continue;
    users.emplace_back(WhoUser(client.User(), client.Data().State(), client.IdleTime(), 
                       client.CurrentCommand(), client.Ident(), client.Address()));
  }
  
  promise.set_value(true);
}

void ReloadConfig::Execute(Listener&)
{
  try
  {
    LoadConfig();
  }
  catch (const cfg::ConfigError& e)
  {
    logs::error << "Failed to load config: " + e.Message() << logs::endl;
    promise.set_value(Result::Fail);
    return;
  }
  
  if (cfg::RequireStopStart()) promise.set_value(Result::StopStart);
  else promise.set_value(Result::Okay);
}

}
}
