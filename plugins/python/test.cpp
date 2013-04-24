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

#include <iostream>
#include <functional>

template <typename R, typename... Args>
struct Wrapper
{
  typedef R(*Function)(Args...);
  Function func;

  Wrapper(Function func) : func(func) { }

  R operator()(Args&&... args)
  {
    std::cout << "before" << std::endl;
    func(std::forward<Args>(args)...);
    std::cout << "after" << std::endl;
  }
};

template <typename R, typename... Args>
std::function<R(Args...)> MakeWrapper(R(*function)(Args...))
{
  return Wrapper<R, Args...>(function);
}

struct SomeClass
{
  void MemberFunc() { }
  void MemberFunc() const { }
};

int main()
{
  std::function<void(const SomeClass*) const> f = &SomeClass::MemberFunc;

  const SomeClass sc;

  f(&sc);
}
