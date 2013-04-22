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

#ifndef __FTP_UTIL_HPP
#define __FTP_UTIL_HPP

#include <vector>
#include <string>
#include <sys/types.h>

namespace ftp
{

void CRLFtoLF(const char* source, size_t len, std::vector<char>& dest);
void LFtoCRLF(const char* source, size_t len, std::vector<char>& dest);

inline void ASCIITranscodeRETR(const char* source, size_t len, std::vector<char>& asciiBuf)
{
  LFtoCRLF(source, len, asciiBuf);
}

inline void ASCIITranscodeSTOR(const char* source, size_t len, std::vector<char>& asciiBuf)
{
#if defined(__CYGWIN__) || defined(_WIN32) || defined(__WIN64)
  LFtoCRLF(source, len, asciiBuf);
#else
  CRLFtoLF(source, len, asciiBuf);
#endif
}

inline void StripTelnetChars(std::string& commandLine)
{
  auto it = commandLine.begin();
  for (; it != commandLine.end(); ++it)
  {
    if (static_cast<unsigned char>(*it) <= 240) break;
  }
  
  commandLine.erase(commandLine.begin(), it);
}

} /* ftp namespace */

#endif
