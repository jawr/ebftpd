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

#ifndef __UTIL_SCOPEGUARD_HPP
#define __UTIL_SCOPEGUARD_HPP

#include <functional>
#include <utility>
#include <stdexcept>

namespace util
{

template <typename Lambda>
class ScopeExit
{
  Lambda cleanup;
  bool cleared;
  
public:
  ScopeExit& operator=(ScopeExit&& rhs)
  {
    cleanup = std::move(rhs.cleanup);
    cleared = rhs.cleared;
    rhs.cleared = true;
  }

  ScopeExit(ScopeExit&& other) :
    cleanup(std::move(other.cleanup)),
    cleared(other.cleared)
  {
    other.cleared = true;
  }

  ScopeExit(Lambda&& cleanup) :
    cleanup(std::forward<Lambda>(cleanup)),
    cleared(false)
  { }
  
  virtual ~ScopeExit()
  {
    try
    {
      if (!cleared)
        cleanup();
    }
    catch (...)
    {
    }
  }
  
  void Clear() { cleared = true; }
};

template <typename Lambda>
class ScopeSuccess : public ScopeExit<Lambda>
{
public:
  ScopeSuccess& operator=(ScopeSuccess&&) = default;
  ScopeSuccess(ScopeSuccess&&) = default;

  ScopeSuccess(Lambda&& cleanup) :
    ScopeExit<Lambda>(std::forward<Lambda>(cleanup))
  { }
  
  ~ScopeSuccess()
  {
    if (std::uncaught_exception()) this->Clear();
  }
};

template <typename Lambda>
class ScopeError : public ScopeExit<Lambda>
{
public:
  ScopeError& operator=(ScopeError&&) = default;
  ScopeError(ScopeError&&) = default;
  
  ScopeError(Lambda&& cleanup) :
    ScopeExit<Lambda>(std::forward<Lambda>(cleanup))
  { }
  
  ~ScopeError()
  {
    if (!std::uncaught_exception()) this->Clear();
  }
};


template <typename T1, typename T2 = T1>
class Transaction
{
  T1 oldValue;
  T1& var;
  bool rollback;
  
public:
  Transaction(T1& var) :
    oldValue(var),
    var(var),
    rollback(false)
  { }
  
  Transaction(T1& var, T2&& newValue) :
    oldValue(std::move(var)),
    var(var),
    rollback(false)
  {
    var = std::forward<T2>(newValue);
  }
  
  ~Transaction()
  {
    try
    {
      if (rollback || std::uncaught_exception())
      {
        std::swap(var, oldValue);
      }
    }
    catch (...)
    {
    }
  }
  
  void Rollback() { rollback = true; }
};

template <typename Lambda>
ScopeExit<Lambda> MakeScopeExit(Lambda&& cleanup)
{
  return ScopeExit<Lambda>(std::forward<Lambda>(cleanup));
}

template <typename Lambda>
ScopeSuccess<Lambda> MakeScopeSuccess(Lambda&& cleanup)
{
  return ScopeSuccess<Lambda>(std::forward<Lambda>(cleanup));
}

template <typename Lambda>
ScopeError<Lambda> MakeScopeError(Lambda&& cleanup)
{
  return ScopeError<Lambda>(std::forward<Lambda>(cleanup));
}

template <typename T1, typename T2>
Transaction<T1, T2> MakeTransaction(T1& var, T2&& newValue)
{
  return Transaction<T1, T2>(var, std::forward<T2>(newValue));
}

template <typename T1>
Transaction<T1> MakeTransaction(T1& var)
{
  return Transaction<T1>(var);
}

} /* util namespace */

#endif
