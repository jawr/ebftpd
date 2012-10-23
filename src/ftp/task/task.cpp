#include "ftp/task/task.hpp"

namespace ftp { namespace task
{

void KickUser::Execute(Listener& listener)
{
  for (auto& client: listener.clients)
  {
    if (client.User().UID() == uid)
      client.SetFinished(); 
  }
}

// end
}
}
