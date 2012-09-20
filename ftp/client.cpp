#include "ftp/client.hpp"
#include "logger/logger.hpp"
namespace ftp
{
void Client::Run()
{
  logger::ftpd << "Starting client" << logger::endl;
}
}
