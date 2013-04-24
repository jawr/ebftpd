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

#ifndef __EXEC_READER_HPP
#define __EXEC_READER_HPP

#include <string>
#include <vector>

namespace ftp
{
class Client;
}

namespace exec
{

class Reader
{
  ftp::Client& client;
  bool open;
  std::string file;
  std::vector<std::string> argv;
  
public:
  Reader(ftp::Client& client, const std::vector<std::string>& argv);
      
  ~Reader();
      
  bool Getline(std::string& line);
  void Close();
  int ExitStatus();
};

} /* exec namespace */

#endif
