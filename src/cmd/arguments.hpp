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

#ifndef __CMD_ARGUMENTS_HPP
#define __CMD_ARGUMENTS_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include "util/string.hpp"
#include "cmd/error.hpp"

namespace cmd
{

class ArgumentParser;

class Arguments
{
  std::vector<std::string> args;
  std::vector<std::string> names;

  void Push(const std::string& name, std::string&& arg)
  {
    args.emplace_back(arg);
    names.emplace_back(name);
  }
  
public:
  typedef std::vector<std::string>::allocator_type allocator_type;
  typedef std::vector<std::string>::size_type size_type;
  typedef std::vector<std::string>::difference_type difference_type;
  typedef std::vector<std::string>::value_type value_type;
  typedef std::vector<std::string>::pointer pointer;
  typedef std::vector<std::string>::const_pointer const_pointer;
  typedef std::vector<std::string>::reference reference;
  typedef std::vector<std::string>::const_reference const_reference;
  typedef std::vector<std::string>::iterator iterator;
  typedef std::vector<std::string>::const_iterator const_iterator;
  typedef std::vector<std::string>::reverse_iterator reverse_iterator;
  typedef std::vector<std::string>::const_reverse_iterator const_reverse_iterator;

  const std::vector<std::string>& Names() const { return names; }
  
  const std::string& operator[](size_t i) const { return args[i]; }
  std::string& operator[](size_t i) { return args[i]; }
  
  const std::string& at(size_t i) const
  {
    if (i >= args.size()) throw std::out_of_range("out of range");
    return operator[](i);
  }
  
  std::string& at(size_t i)
  {
    if (i >= args.size()) throw std::out_of_range("out of range");
    return operator[](i);
  }
  
  std::string& front() { return args.front(); }
  const std::string& front() const { return args.front(); }
  
  std::string& back() { return args.back(); }
  const std::string& back() const { return args.back(); }

  const std::string& operator[](const std::string& name) const
  {
    auto it = std::find(names.begin(), names.end(), name);
    if (it == names.end()) throw std::out_of_range("Argument with name doesn't exist");
    return operator[](std::distance(names.begin(), it));
  }
  
  std::string& operator[](const std::string& name)
  {
    auto it = std::find(names.begin(), names.end(), name);
    if (it == names.end()) throw std::out_of_range("Argument with name doesn't exist");
    return operator[](std::distance(names.begin(), it));
  }
  
  iterator begin() { return args.begin(); }
  iterator end() { return args.end(); }
  const_iterator begin() const { return args.cbegin(); }
  const_iterator end() const { return args.cend(); }
  const_iterator cbegin() const { return args.cbegin(); }
  const_iterator cend() const { return args.cend(); }
  reverse_iterator rbegin() { return args.rbegin(); }
  reverse_iterator rend() { return args.rend(); }
  const_reverse_iterator rbegin() const { return args.rbegin(); }
  const_reverse_iterator rend() const { return args.rend(); }
  const_reverse_iterator crbegin() const { return args.rbegin(); }
  const_reverse_iterator crend() const { return args.rend(); }
  
  size_type size() const { return args.size(); }
  bool empty() const { return args.empty(); }
  
  void swap(Arguments& a)
  {
    args.swap(a.args);
    names.swap(a.names);
  }
  
  friend class ArgumentParser;
};

class ArgumentParser
{
  enum class Type
  {
    Single,
    Grouped,
    Multiple
  };

  std::vector<std::pair<std::string, Type>> types;

  void Compile(const std::string& grammar);
  Arguments Parse(const std::string& line);
  
public:
  ArgumentParser(const std::string& grammar)
  {
    Compile(grammar);
  }

  Arguments operator()(const std::string& line)
  {
    return Parse(line);
  }
};

std::string ExtractCommand(const std::string& line);

} /* cmd namespace */

#endif
