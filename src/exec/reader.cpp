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
    logs::Error("Error while closing child process executed by %1%: %2%", 
                client.User().Name(), boost::join(argv, " "));
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
      logs::Debug("Child process executed by %1% needed SIGTERM to be closed: %2%",
                  client.User().Name(), boost::join(argv, " "));
      return;
    }
    
    if (client.Child().Kill(SIGKILL, util::TimePair(1, 0)))
    {
      logs::Error("Child process executed by %1% needed SIGKILL to be closed: %2%",
                  client.User().Name(), boost::join(argv, " "));
    }
    else
    {
      logs::Error("Child process executed by %1% failed to close even with SIGKILL: ",
                  client.User().Name(), boost::join(argv, " "));
    }
    open = false;
  }
}

int Reader::ExitStatus()
{
  return client.Child().ExitStatus();
}

} /* exec namespace */
