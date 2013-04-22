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

#include "ftp/util.hpp"

namespace ftp {

void LFtoCRLF(const char* source, size_t len, std::vector<char>& dest)
{
  dest.reserve(len * 2);
  dest.clear();
  
  for (size_t i = 0; i < len; ++i)
  {
    if (source[i] == '\n' && i != 0 && source[i - 1] != '\r') dest.emplace_back('\r');
    dest.emplace_back(source[i]);
  }
}

void CRLFtoLF(const char* source, size_t len, std::vector<char>& dest)
{
  dest.reserve(len);
  dest.clear();
  
  for (size_t i = 0; i < len; ++i)
  {
    if (source[i] != '\r') dest.emplace_back(source[i]);
  }
}

} /* ftp namespace */
