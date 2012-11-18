#include <boost/algorithm/string/join.hpp>
#include "exec/reader.hpp"
#include "util/processreader.hpp"
#include "ftp/client.hpp"
#include "exec/util.hpp"
#include "logs/logs.hpp"

namespace exec
{

Reader::Reader(ftp::Client& client, const std::vector<std::string>& argv) :
  client(client), open(false)
{
  util::ProcessReader::ArgvType env;
  client.Child().Open(argv[0], argv, BuildEnv(client));
  open = true;
}

Reader::~Reader()
{
  try
  {
    Close();
  }
  catch (const util::SystemError& e)
  {
    logs::error << "Error while closing child process executed by " << client.User().Name() 
                << ": " << boost::join(argv, " ") << logs::endl;
  }
}

bool Reader::Getline(std::string& line)
{
  return client.Child().Getline(line);
}

void Reader::Close()
{
  if (open)
  {
    if (client.Child().Close(util::TimePair(1, 0))) return;
    if (client.Child().Kill(util::TimePair(1, 0)))
    {
      logs::debug << "Child process executed by " << client.User().Name() 
                  << " needed SIGTERM to be closed: " 
                  << boost::join(argv, " ") << logs::endl;
      return;
    }
    
    if (client.Child().Kill(SIGKILL, util::TimePair(1, 0)))
    {
      logs::error << "Child process executed by " << client.User().Name() 
                  << " needed SIGKILL to be closed: " 
                  << boost::join(argv, " ") << logs::endl;
    }
    else
    {
      logs::error << "Child process executed by " << client.User().Name() 
                  << " failed to close even with SIGKILL: " 
                  << boost::join(argv, " ") << logs::endl;
    }
    open = false;
  }
}

} /* exec namespace */
