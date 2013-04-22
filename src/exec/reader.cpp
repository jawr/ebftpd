//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "util/string.hpp"
#include "exec/reader.hpp"
#include "util/processreader.hpp"
#include "ftp/client.hpp"
#include "exec/util.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"

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
                client.User().Name(), util::Join(argv, " "));
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
                  client.User().Name(), util::Join(argv, " "));
      return;
    }
    
    if (client.Child().Kill(SIGKILL, util::TimePair(1, 0)))
    {
      logs::Error("Child process executed by %1% needed SIGKILL to be closed: %2%",
                  client.User().Name(), util::Join(argv, " "));
    }
    else
    {
      logs::Error("Child process executed by %1% failed to close even with SIGKILL: ",
                  client.User().Name(), util::Join(argv, " "));
    }
    open = false;
  }
}

int Reader::ExitStatus()
{
  return client.Child().ExitStatus();
}

} /* exec namespace */
