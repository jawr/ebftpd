#include <sstream>
#include "ftp/task/task.hpp"
#include "ftp/listener.hpp"
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
  boost::posix_time::ptime now(boost::posix_time::microsec_clock::local_time());
  
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
    cfg::UpdateShared(std::shared_ptr<cfg::Config>(new cfg::Config(configFile)));
  }
  catch (const cfg::ConfigError&)
  {
    promise.set_value(false);
    return;
  }
  
  // threads need to be notified
  
  promise.set_value(true);
}

}
}
