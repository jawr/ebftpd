#ifndef __FTP_LISTENER_HPP
#define __FTP_LISTENER_HPP
#include <boost/thread.hpp>
#include "util/endpoint.hpp"
namespace ftp
{
class Listener
{
  boost::thread thread;
  ::util::endpoint addr;
public:
  Listener();
  Listener(const std::string& ip, uint16_t port);
  ~Listener();
  void Listen();
  void Main();
};
}
#endif
