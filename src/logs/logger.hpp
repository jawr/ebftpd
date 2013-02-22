#ifndef __LOGS_LOGGER_HPP
#define __LOGS_LOGGER_HPP

#include <memory>
#include <vector>
#include <string>
#include "logs/sink.hpp"

namespace logs
{

struct Quote
{
  char ch;
  Quote(char ch = '"') : ch(ch) { }
};

struct QuoteOn : public Quote
{
  QuoteOn(char ch = '"') : Quote(ch) { }
};

struct QuoteOff : public Quote
{
  QuoteOff() : Quote('\0') { }
};

class Logger
{
  enum class QuoteMode
  {
    On,
    Next,
    Off
  };

  std::vector<std::shared_ptr<Sink>> sinks;
  QuoteMode quoteMode;
  char quoteChar;
  
public:
  Logger() : quoteMode(QuoteMode::Off), quoteChar('\0') { }
  
  void PushSink(const std::shared_ptr<Sink>& sink)
  {
    sinks.emplace_back(sink);
  }
  
  void Write()
  {
    for (auto& sink : sinks)
    {
      sink->Flush();
    }
  }
  
  template <typename... Args>
  void Write(const char* field, const QuoteOn& quote, const Args&... args)
  {
    quoteMode = QuoteMode::On;
    quoteChar = quote.ch;
    Write(field, args...);
  }

  template <typename... Args>
  void Write(const char* field, const QuoteOff& /* quote */, const Args&... args)
  {
    quoteMode = QuoteMode::Off;
    quoteChar = '\0';
    Write(field, args...);
  }

  template <typename... Args>
  void Write(const char* field, const Quote& quote, const Args&... args)
  {
    quoteMode = QuoteMode::Next;
    quoteChar = quote.ch;
    Write(field, args...);
  }

  template <typename... Args>
  void Write(const QuoteOn& quote, const Args&... args)
  {
    quoteMode = QuoteMode::On;
    quoteChar = quote.ch;
    Write(args...);
  }

  template <typename... Args>
  void Write(const QuoteOff& /* quote */, const Args&... args)
  {
    quoteMode = QuoteMode::Off;
    quoteChar = '\0';
    Write(args...);
  }

  template <typename... Args>
  void Write(const Quote& quote, const Args&... args)
  {
    quoteMode = QuoteMode::Next;
    quoteChar = quote.ch;
    Write(args...);
  }
  
  template <typename Value, typename... Args>
  void Write(const char* field, const Value& value, const Args&... args)
  {
    for (auto& sink : sinks)
    {
      sink->QuoteNext(quoteChar);
      sink->Write(field, value);
    }
        
    if (quoteMode == QuoteMode::Next)
    {
      quoteMode = QuoteMode::Off;
      quoteChar = '\0';
    }

    Write(args...);
  }
  
  template <typename Value, typename... Args>
  void Write(const std::string& field, const Value& value, const Args&... args)
  {
    Write(field.c_str(), value, args...);
  }
};

} /* logs namespace */

#endif
