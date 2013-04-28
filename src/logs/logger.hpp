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

#ifndef __LOGS_LOGGER_HPP
#define __LOGS_LOGGER_HPP

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include "logs/sink.hpp"

namespace logs
{

struct Quote
{
  char ch;
  Quote(char ch = '"') : ch(ch) { }
};

struct QuoteOn
{
  char ch;
  QuoteOn(char ch = '"') : ch(ch) { }
};

struct QuoteOff
{
  char ch;
  QuoteOff() : ch('\0') { }
};

struct Tag { };

struct Brackets
{
  std::pair<char, char> ch;
  Brackets(char open, char close) :
    ch{open, close}
  { }
};

class Logger
{
  enum class QuoteMode { On, Next, Off };
  enum class TagMode { Next, Off };
  enum class BracketMode { Next, Off };
  
  struct Entry
  {
    QuoteMode quoteMode;
    char quoteChar;
    TagMode tagMode;
    BracketMode bracketMode;
    std::pair<char, char> bracketChar;
    const char* field;
    
    Entry() : 
      quoteMode(QuoteMode::Off), 
      quoteChar('\0'), 
      tagMode(TagMode::Off),
      bracketMode(BracketMode::Off),
      bracketChar{ '\0', '\0' },
      field(nullptr) 
    { }
  };

  std::vector<std::shared_ptr<Sink>> sinks;
  
  void Flush()
  {
    for (auto& sink : sinks)
    {
      sink->Flush();
    }
  }
  
  template <typename Value, typename... Args>
  void Write(Entry& entry, const Value& value)
  {
    bool tag = entry.tagMode == TagMode::Next;
    
    for (auto& sink : sinks)
    {
      sink->Formatting(tag, entry.quoteChar, entry.bracketChar);
      sink->Write(entry.field, value);
    }
        
    if (entry.quoteMode == QuoteMode::Next)
    {
      entry.quoteMode = QuoteMode::Off;
      entry.quoteChar = '\0';
    }
    
    entry.tagMode = TagMode::Off;
    
    if (entry.bracketMode == BracketMode::Next)
    {
      entry.bracketMode = BracketMode::Off;
      entry.bracketChar = { '\0', '\0' };
    }
  }

  void Parse(Entry& entry)
  {
    if (entry.field) throw std::logic_error("All log fields must be in name / value pairs");
    Flush();
  }
  
  template <typename... Args>
  void Parse(Entry& entry, const Brackets& arg, const Args&... args)
  {
    entry.bracketMode = BracketMode::Next;
    entry.bracketChar = arg.ch;
    Parse(entry, args...);
  }
  
  template <typename... Args>
  void Parse(Entry& entry, const Tag& /* arg */, const Args&... args)
  {
    entry.tagMode = TagMode::Next;
    Parse(entry, args...);
  }
  
  template <typename... Args>
  void Parse(Entry& entry, const Quote& arg, const Args&... args)
  {
    entry.quoteMode = QuoteMode::Next;
    entry.quoteChar = arg.ch;
    Parse(entry, args...);
  }
  
  template <typename... Args>
  void Parse(Entry& entry, const QuoteOn& arg, const Args&... args)
  {
    entry.quoteMode = QuoteMode::On;
    entry.quoteChar = arg.ch;
    Parse(entry, args...);
  }
  
  template <typename... Args>
  void Parse(Entry& entry, const QuoteOff& /* arg */, const Args&... args)
  {
    entry.quoteMode = QuoteMode::Off;
    entry.quoteChar = '\0';
    Parse(entry, args...);
  }
  
  template <typename... Args>
  void Parse(Entry& entry, const char* arg, const Args&... args)
  {
    if (entry.field)
    {
      Write(entry, arg);
      entry.field = nullptr;
    }
    else
    {
      entry.field = arg;
    }
    Parse(entry, args...);
  }
  
  template <typename... Args>
  void Parse(Entry& entry, const std::string& arg, const Args&... args)
  {
    Parse(entry, arg.c_str(), args...);
  }
  
  template <typename T, typename... Args>
  void Parse(Entry& entry, const T& arg, const Args&... args)
  {
    if (!entry.field) throw std::logic_error("All log fields must be in name / value pairs");
    Write(entry, arg);
    entry.field = nullptr;
    Parse(entry, args...);
  }
  
public:
  void PushSink(const std::shared_ptr<Sink>& sink)
  {
    sinks.emplace_back(sink);
  }
    
  template <typename... Args>
  void PushEntry(const Args&... args)
  {
    Entry entry;
    Parse(entry, args...);
  }
  
  // utility function only for dynamic event logging
  void PushEvent(const std::string& what, 
                 const std::vector<std::pair<std::string, std::string>>& pairs);
};

} /* logs namespace */

#endif
