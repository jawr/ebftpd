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

#include "logs/logger.hpp"

namespace logs
{

void Logger::PushEvent(const std::string& what, 
                       const std::vector<std::pair<std::string, std::string>>& pairs)
{
  if (pairs.empty()) return;
  Entry entry;
  entry.tagMode = TagMode::Next;
  entry.field = "event";
  Write(entry, what.c_str());

  entry.quoteMode = QuoteMode::On;
  entry.quoteChar = '"';
  for (const auto& kv : pairs)
  {
    entry.field = kv.first.c_str();
    Write(entry, kv.second.c_str());
  }
  Flush();
}

} /* logs namespace */
