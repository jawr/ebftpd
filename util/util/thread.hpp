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

#ifndef __UTIL_THREAD_HPP
#define __UTIL_THREAD_HPP

#include <boost/thread/thread.hpp>

namespace util
{

class Thread
{
  void Main();

protected:
  boost::thread thread;
  bool started;
  
  virtual void Run() = 0;

public:
  Thread() : started(false) { }
  virtual ~Thread() { }

  void Start();
  void Join();
  bool TryJoin();
  void Stop(bool join = false);
  bool Started() const { return started; }
};

// end util namespace
}

#endif
