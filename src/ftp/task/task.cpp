#include <sstream>
#include "ftp/task/task.hpp"
#include "ftp/listener.hpp"

namespace ftp { namespace task
{

void KickUser::Execute(Listener& listener)
{
  for (auto& client: listener.clients)
  {
    if (client.User().UID() == uid)
      client.SetState(ClientState::Finished); 
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

// end
}
}
