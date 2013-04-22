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

#include <unistd.h>
#include <exception>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "util/thread.hpp"
#include "util/error.hpp"

namespace util
{

void Thread::Start()
{ 
  assert(!started);
  thread = boost::thread(&Thread::Main, this);  
  started = true;
}

void Thread::Stop(bool join)
{
  if (started)
  {
    thread.interrupt();
    if (join)
    {
      thread.join();
      started = false;
    }
  }
}

void Thread::Join()
{
  if (started) 
  {
    thread.join();
    started = false;
  }
}

bool Thread::TryJoin()
{
  if (!started) return true;
  if (thread.timed_join(boost::posix_time::seconds(0)))
  {
    started = false;
    return true;
  }
  return false;
}

void Thread::Main()
{
  try
  {
    Run();
  }
  catch (const boost::thread_interrupted&)
  {
  }
}

// end util namespace
}
