#ifndef __FTP_LISTENER_HPP
#define __FTP_LISTENER_HPP
#include <boost/thread.hpp>
#include "util/endpoint.hpp"
#include "util/thread.hpp"
namespace ftp
{
class Listener : public util::Thread
{
  boost::thread thread;
  ::util::endpoint addr;
public:
  Listener() : Thread(), addr("127.0.0.1", 21) {};
  Listener(const std::string& ip, uint16_t port) : Thread(), addr(ip, port) {};
  ~Listener() {};
  void Run();
};
}
#endif
