#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include "ftp/listener.hpp"
#include "ftp/client.hpp"
#include "logger/logger.hpp"
#include <iostream>
namespace ftp
{
void Listener::Run()
{
  logger::ftpd << "Starting listener on " << addr.ip() << ":" << addr.port() << logger::endl;
  server.listen(addr);
  while (true)
  {
    // needs to move to some kind of pool. or at least a manager.
    // colud use a similar pool to what i had before, but this requires a
    // global object.
    ftp::Client *client = new ftp::Client();
    server.accept(client->Socket());
    logger::ftpd << "Got client: " << client->Socket().remote_endpoint().ip() << logger::endl;
    clients.insert(client);
  }
}

}
// end ftp namespace

#ifdef LISTENER_TEST
int main()
{
  ftp::Listener l;
  l.Start();
  l.Join();
  return 0;
}
#endif
