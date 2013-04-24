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

#ifndef __SIGNAL__SIGNAL_HPP
#define __SIGNAL__SIGNAL_HPP

#include <string>
#include <memory>
#include "util/thread.hpp"
#include "util/error.hpp"

namespace signals
{

class Handler : public util::Thread
{
  void Run();
  
  static std::unique_ptr<Handler> instance;
  
public:
  static void StartThread();
  static void StopThread();
};


util::Error Initialise(const std::string& logPath);

} /* signals namespace */

#endif
