#include <sstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "util/net/identclient.hpp"
#include "util/net/tcpsocket.hpp"

namespace util { namespace net
{

IdentClient::IdentClient(const Endpoint& localEndpoint,
                         const Endpoint& remoteEndpoint) :
  socket(Endpoint(remoteEndpoint.IP(), identPort)),
	localEndpoint(localEndpoint),
	remoteEndpoint(remoteEndpoint)
{
  Request();
}

IdentClient::IdentClient(const TCPSocket& client) :
	socket(Endpoint(client.RemoteEndpoint().IP(), identPort)),
  localEndpoint(client.LocalEndpoint()),
  remoteEndpoint(client.RemoteEndpoint())
{
  Request();
}

void IdentClient::Request(const Endpoint& localEndpoint,
                          const Endpoint& remoteEndpoint)
{
  this->localEndpoint = localEndpoint;
  this->remoteEndpoint = remoteEndpoint;
  socket.Connect(Endpoint(remoteEndpoint.IP(), identPort));
  Request();
}

void IdentClient::Request()
{
  std::ostringstream buf;
  buf << remoteEndpoint.Port() << ", " << localEndpoint.Port() << "\r\n";
  socket.Write(buf.str().c_str(), buf.str().length());
  
  std::string response;
  socket.Getline(response, true);

  std::vector<std::string> toks;
  boost::split(toks, response, boost::is_any_of(":"));
  if (toks.size() < 3) throw NetworkError("Malformed ident response");
  
  boost::trim(toks[0]);
  std::stringstream is(toks[0]);

  uint16_t remotePort;
  is >> remotePort;
  char comma;
  is >> comma;
  uint16_t localPort;
  is >> localPort;
  
  if (!is.eof() || comma != ',')
    throw NetworkError("Malformed ident response");
    
  boost::to_upper(toks[1]);
  boost::trim(toks[1]);
  if (toks[1] == "ERROR") throw NetworkError("Ident error: " + toks[2]);
  if (toks[1] != "USERID" || toks.size() != 4)
    throw NetworkError("Malformed ident response");
  
  os = boost::trim_copy(toks[2]);
  ident = boost::trim_copy(toks[3]);
}

} /* net namespace */
} /* util namespace */

#ifdef UTIL_NET_IDENTCLIENT_TEST

#include <iostream>

int main()
{
  using namespace util::net;

  IdentClient ident;
  try
  {
    ident.Request(Endpoint("127.0.0.1", 3456), Endpoint("127.0.0.1", 543));
  }
  catch (const NetworkError& e)
  {
    std::cout << e.Message() << std::endl;
    return 0;
  }

  
  std::cout << ident.Ident() << " " << ident.OS() << std::endl;
}

#endif
