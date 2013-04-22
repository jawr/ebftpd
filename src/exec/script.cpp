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

#include <string>
#include "script.hpp"
#include "ftp/client.hpp"
#include "fs/path.hpp"
#include "exec/reader.hpp"

namespace exec
{

int Script(ftp::Client& client, const util::ProcessReader::ArgvType& argv, std::string& messages)
{
  exec::Reader reader(client, argv);
  messages.clear();

  std::string line;
  while (reader.Getline(line))
  {
    if (!messages.empty()) messages += '\n';
    messages += line;
  }
  
  reader.Close();
  return reader.ExitStatus();
}

} /* exec namespace */
