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

#include <cctype>
#include <sys/stat.h>
#include "fs/mode.hpp"
#include "cfg/get.hpp"

// this is based on chmod in the gnu coreutils package

namespace fs
{

mode_t NumericModeFromString(const std::string& str)
{
  if (str.empty() || 
      (str.length() > 3 &&
       (str.length() != 4 || str[0] != '0'))) throw InvalidModeString();

  mode_t value = 0;
  unsigned factor = 1;
  for (auto it = str.rbegin(); it != str.rend(); ++it)
  {
    if (*it < '0' || *it > '7') throw InvalidModeString();
    value += (*it - '0') * factor;
    factor *= 8;
  }

  return value;
}

void Mode::CompileNumericMode(const std::string& str)
{
  changes.emplace_back(Operator::Equals, 
      ::fs::Mode::Type::Normal, S_IRWXU | S_IRWXG | S_IRWXO, NumericModeFromString(str));
}

void Mode::CompileSymbolicMode(const std::string& str)
{
  auto it = str.begin();
  auto end = str.end();
  while (it != end)
  {
    mode_t affected = 0;
    for (; it != end; ++it)
    {
      if (*it == 'a') affected |= S_IRWXU | S_IRWXG | S_IRWXO;
      else if (*it == 'u') affected |= S_IRWXU;
      else if (*it == 'g') affected |= S_IRWXG;
      else if (*it == 'o') affected |= S_IRWXO;
      else break;
    }
    
    do
    {
      Operator op;
      ::fs::Mode::Type type = ::fs::Mode::Type::Copy;
      if (*it == '+') op = Operator::Plus;
      else if (*it == '-') op = Operator::Minus;
      else if (*it == '=') op = Operator::Equals;
      else throw InvalidModeString();
      
      mode_t value = 0;      
      if (++it == end) type = ::fs::Mode::Type::Normal;
      else
      {
        if (*it == 'u') { value |= S_IRWXU; ++it; }
        else if (*it == 'g') { value |= S_IRWXG; ++it; }
        else if (*it == 'o') { value |= S_IRWXO; ++it; }
        else
        {
          type = ::fs::Mode::Type::Normal;
          for (; it != end; ++it)
          {
            if (*it == 'r') value |= S_IRUSR | S_IRGRP | S_IROTH;
            else if (*it == 'w') value |= S_IWUSR | S_IWGRP | S_IWOTH;
            else if (*it == 'x') value |= S_IXUSR | S_IXGRP | S_IXOTH;
            else break;
          }
        }
      }
      changes.emplace_back(op, type, affected, value);
    }
    while (*it == '-' || *it == '=' || *it == '+');
    if (*it != ',') break;
    ++it;
  }

  if (it != end) throw InvalidModeString();
}

void Mode::Compile(const std::string& str)
{
  if (str.empty()) throw InvalidModeString();
  if (std::isdigit(str[0])) CompileNumericMode(str);
  else CompileSymbolicMode(str);
}

void Mode::Apply(mode_t oldMode, mode_t umask, mode_t& newMode) const
{
  newMode = oldMode;
  for (auto& c : changes)
  {
    mode_t value = c.Value();
    if (c.Type() == ::fs::Mode::Type::Copy)
    {
      value &= newMode;
      if (value & (S_IRUSR | S_IRGRP | S_IROTH))
        value |= S_IRUSR | S_IRGRP | S_IROTH;
      if (value & (S_IWUSR | S_IWGRP | S_IWOTH))
        value |= S_IWUSR | S_IWGRP | S_IWOTH;
      if (value & (S_IXUSR | S_IXGRP | S_IXOTH))
        value |= S_IXUSR | S_IXGRP | S_IXOTH;
    }
    
    if (c.Affected()) value &= c.Affected();
    else value &= ~umask;
    
    switch (c.Op())
    {
      case Operator::Equals :
        newMode = (newMode & ~c.Affected()) | value;
        break;
      case Operator::Plus   :
        newMode |= value;
        break;
      case Operator::Minus  :
        newMode &= ~value;
        break;
    }
  }
}

void InitialiseUmask()
{
 cfg::ConnectUpdatedSlot([]() { umask(cfg::Get().Umask()); });
}

mode_t CurrentUmask()
{
  mode_t mask;
  umask(mask = umask(0));
  return mask;
}

} /* fs namespace */
