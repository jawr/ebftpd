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
  for (auto& client: listener.clients)
  {
    if (client.State() == ClientState::LoggedIn)
    {
      users.emplace_back(WhoUser(client.User(), client.IdleTime(), 
        client.CurrentCommand(), client.Ident(), client.Address()));
    }
  }
  promise.set_value(true);
}

// end
}
}
