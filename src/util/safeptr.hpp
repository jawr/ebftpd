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
