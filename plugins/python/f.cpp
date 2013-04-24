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

#include <utility>
#include <type_traits>
#include <iostream>

template <class Fn, class Arg>
struct get_return_type
{
    typedef decltype( std::declval<Fn>() ( std::declval<Arg>() ) ) type;
};

template <class Fn>
struct wrapper
{
    explicit wrapper(Fn fn) : fn(fn) {}
    Fn fn;

    template <class Arg>
    typename get_return_type<Fn,Arg&&>::type
    operator()(Arg&& arg)
    {
        return fn(std::forward<Arg>(arg));
    }

    template <class Arg>
    typename get_return_type< const Fn,Arg&&>::type
    operator()(Arg&& arg) const
    {
        return fn(std::forward<Arg>(arg));
    }
};

struct x {};
struct y {};

struct foo
{
    x* operator()(x) { std::cout << "x* operator()(x)" << std::endl; return nullptr; }
    x operator()(x) const { std::cout << "x operator()(x) const" << std::endl; return x(); }
    y* operator()(y) { std::cout << "y* operator()(y)" << std::endl; return nullptr; }
    y operator()(y) const { std::cout << "y operator()(y) const" << std::endl; return y(); }
};

template <class Fn>
void test_foo(Fn fn)
{
    // make sure all operator() overloads are callable
    const Fn& cfn = fn;

    x* a = fn(x());
    x b = cfn(x());
    y* c = fn(y());
    y d = cfn(y());
(void)a;(void)b;(void)c;(void)d;
}

int main()
{
    test_foo(foo());
    test_foo(wrapper<foo>(foo())); // fail
}
