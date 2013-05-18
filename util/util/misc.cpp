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

#include <iomanip>
#if defined(__linux__)
#include <sys/prctl.h>
#else
#include <unistd.h>
#endif
#include "util/misc.hpp"

namespace util
{

void SetProcessTitle(const std::string& title)
{
#if defined(__linux__)
  prctl(PR_SET_NAME, title.c_str(), 0, 0, 0);
#else
  setproctitle(title.c_str());
#endif
}

std::string EscapeURI(const std::string& uri)
{
	std::ostringstream os;
	for (char ch : uri)
	{
		if (std::isalnum(ch) || ch == '-' || ch == '.' || ch == '_' || ch == '~')
		{
			os << ch;
		}
		else
		{
			os << '%' << std::setw(2) << std::uppercase << std::hex << static_cast<unsigned int>(ch);
		}
	}
	return os.str();
}

} /* util namespace */
