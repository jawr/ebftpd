#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "ftp/client.hpp"
#include "logger/logger.hpp"
#include "util/tcpserver.hpp"
#include "util/exception.hpp"
#include "util/verify.hpp"
#include "util/error.hpp"
#include "util/scopeguard.hpp"

namespace ftp
{

bool Client::Finished() const
{
  boost::lock_guard<boost::mutex> lock(mutex);
  return finished;
}

void Client::SetFinished()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  finished = true;
}

bool Client::Accept(util::tcp::server& server)
{
  try
  {
    server.accept(socket);
  }
  catch(const util::network_error& e)
  {
    SetFinished();
    logger::error << "Error while acceting new client: " << e.what() << logger::endl;
    return false;
  }
  return true;
}

void Client::SendReply(int code, bool part, const std::string& message)
{
  std::ostringstream reply;
  reply << std::setw(3) << code << (part ? '-' : ' ')  << message << "\r\n";
  const std::string& str = reply.str();
  socket.write(str.c_str(), str.length());
}

void Client::PartReply(int code, const std::string& message)
{
  SendReply(code, true, message);
}

void Client::PartReply(const std::string& message)
{
  verify(lastCode != 0);
  PartReply(lastCode, message);
}

void Client::Reply(int code, const std::string& message)
{
  std::ostringstream reply;
  SendReply(code, false, message);
  lastCode = 0;
}

void Client::Reply(const std::string& message)
{
  verify(lastCode != 0);
  Reply(lastCode, message);
}

void Client::DisplayWelcome()
{
  Reply(220, "Welcome to eyeoh and biohazard's ftpd!");
}

void Client::NextCommand()
{
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(socket.socket(), &readSet);
  FD_SET(interruptPipe[0], &readSet);
  
  struct timeval tv;
  tv.tv_sec = 1800;
  tv.tv_usec = 0;
  
  int max = std::max(socket.socket(), interruptPipe[0]);
  
  int n = select(max + 1, &readSet, NULL, NULL, &tv);
  if (!n) throw util::timeout_error();
  if (n < 0)
  {
    util::Error e = util::Error::Failure(errno);
    throw util::unknown_network_error(e.Message().c_str());
  }
  
  if (FD_ISSET(socket.socket(), &readSet))
  {
    socket.getline(buffer, sizeof(buffer), true);
    command = buffer;
    return;
  }

  boost::this_thread::interruption_point();
  verify(false); // should never get here!!
}

void Client::ExecuteCommand()
{
  std::vector<std::string> args;
  boost::split(args, command, boost::is_any_of(" "),
               boost::token_compress_on);
  if (args.empty()) throw util::network_protocol_error();
  Reply(500, "Commands not implemented yet!");
}

void Client::Handle()
{
  while (!Finished())
  {
    NextCommand();
    ExecuteCommand();
  }
}

void Client::Run()
{
  using util::scope_guard;
  using util::make_guard;
  using boost::bind;
  
  scope_guard finishedGuard = make_guard(bind(&Client::SetFinished, this));
  
  try
  {
    logger::ftpd << "Servicing client connected from "
                 << socket.remote_endpoint() << logger::endl;
  
    DisplayWelcome();
    Handle();
  }
  catch (const util::network_error& e)
  {
    logger::error << "Client from " << socket.remote_endpoint()
                  << " lost connection: " << e.what() << logger::endl;
  }
  
  (void) finishedGuard; /* silence unused variable warning */
}
}
