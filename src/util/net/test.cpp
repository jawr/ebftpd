#ifdef UTIL_NET_TEST

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "util/net/tcplistener.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/net/endpoint.hpp"
#include "util/net/tlscontext.hpp"
#include "util/net/tlserror.hpp"

using namespace util::net;

void ThreadMain()
{
  try
  {
    TCPSocket sock(Endpoint("127.0.0.1", 4567), 5);
    sock.HandshakeTLS(TLSSocket::Client);
    std::cout << "connected" << std::endl;
    if (sock.WaitState(TCPSocket::State(false, true))) sock.Write("hello\n", 6);
    char buf[1024];
    sock.Getline(buf, sizeof(buf));
    std::cout << "client: " << buf << std::endl;
    
    if (sock.WaitState(TCPSocket::State(true, false)))
    {
      std::cout << "state readable" << std::endl;
      sock.Getline(buf, sizeof(buf));
      std::cout << "buf: " << buf << std::endl;
    }
  }
  catch (const NetworkSystemError& e)
  {
    std::cerr << "connect failed: " << e.Message() << std::endl;
  }
  catch (const TLSError& e)
  {
    std::cerr << "TLS: " << e.Message() << std::endl;
  }
}

int main()
{
  try
  {
    TLSClientContext::Initialise();
    TLSServerContext::Initialise("server.pem");
  }
  catch (const TLSError& e)
  {
    std::cout << "TLS: " << e.Message() << std::endl;
    return 1;
  }
  
  TCPListener server(1);
  try
  {
    server.Listen(Endpoint("127.0.0.1", 4567));
  }
  catch (const NetworkSystemError& e)
  {
    std::cerr << "listen failed: " << e.Message() << std::endl;
    return 1;
  }
  
  boost::thread t(&ThreadMain);
  
  TCPSocket client;
  
  try
  {
    server.Accept(client);
    client.HandshakeTLS(TLSSocket::Server);
    char buf[1024];
    client.Getline(buf, sizeof(buf));
    std::cout << "server: " << buf << std::endl;
    client.Write("hello\n", 6);
  }
  catch (const NetworkSystemError& e)
  {
    std::cerr << "accept failed: " << e.Message() << std::endl;
    return 1;
  }
  catch (const TLSError& e)
  {
    std::cerr << "server tls: " << e.Message() << std::endl;
    return 1;
  }

  boost::this_thread::sleep(boost::posix_time::seconds(1));
  
  t.interrupt();
  client.Shutdown();
  
  t.join();
}

#endif
