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

#ifndef __UTIL_SAFEPTR_HPP
#define __UTIL_SAFEPTR_HPP

#include <stdexcept>
#include <memory>

namespace util
{

struct DereferenceNullPtrError : std::logic_error
{
  DereferenceNullPtrError() : std::logic_error("Dereference null pointer error") { }
};

template <
  typename T,
  typename UnderlyingPtr = std::shared_ptr<T>
>
class SafePtr
{
  UnderlyingPtr ptr;
  
public:
  SafePtr& operator=(SafePtr&& rhs) { ptr = std::move(rhs.ptr); }
  SafePtr(SafePtr&& other) : ptr(std::move(other.ptr)) { }
  SafePtr& operator=(const SafePtr& rhs) { ptr = rhs.ptr; }
  SafePtr(const SafePtr& other) : ptr(other.ptr) { }
  SafePtr(T* ptr) : ptr(ptr) { }
  SafePtr() = default;
  
  void Reset(T* ptr) { this->ptr.reset(ptr); }
  
  T& operator*()
  {
    if (!ptr) throw DereferenceNullPtrError();
    return *ptr;
  }
  
  const T& operator*() const
  {
    if (!ptr) throw DereferenceNullPtrError();
    return *ptr;
  }
  
  T* operator->()
  {
    if (!ptr) throw DereferenceNullPtrError();
    return &*ptr;
  }
  
  const T* operator->() const
  {
    if (!ptr) throw DereferenceNullPtrError();
    return &*ptr;
  }
};

} /* util namespace */

#endif
