#include "cmd/arguments.hpp"

namespace cmd
{

/*Initialise      = 0,        
  Single          = 1 << 1,   // combine with notrequired
  Grouped         = 1 << 2,   // combine with notrequired
  Multiple        = 1 << 3,   // combine with notrequired
  NotRequired     = 1 << 4,   // not combine with option
  Option          = 1 << 5,   // combine with optionargument only
  OptionArgument  = 1 << 6    // combine with option only
*/

ArgumentParse::Definition::Definition(const std::string& arg)
{
  std::vector<std::string> toks;
  util::Split(toks, arg, ":");
  if (toks.size() != 2 || toks[0].empty() || toks[1].empty()) 
  {
    throw std::logic_error("Invalid argument grammar: not in format name:type");
  }

  name = toks[0];
  ParseType(toks[1]);
}

bool ArgumentParser::Definition::PushType(Type type)
{
  if (this->type & type == type) return false;  
  switch (type)
  {
    case Type::NotRequired    :
      return this->type & Type::Option != Type::Option;
    case Type::Option         :
      return this->type = Type::Initialise;
    case Type::OptionArgument :
      return this->type == Type::Option;
    case Single               :
    case Grouped              :
    case Multiple             :
      break;
    default                   :
      return false;
  }
  return true;
}

void ArgumentParser::Definition::ParseType(const std::string& type)
{
  for (char ch : type)
  {
    if (!ParseType(static_cast<Type>(ch)))
      throw std::logic_error("Invalid argument grammar: invalid type");
  }
}

void ArgumentParser::Compile(const std::string& grammar)
{
  std::vector<std::string> args;
  util::Split(args, grammar, " ");
  bool haveMultiple = false;
  bool haveNotRequired = false;
  for (std::string& arg : args)
  {
    if (haveMultiple) // multiple must be last argument
    {
      throw std::logic_error("Invalid argument grammar: multiple argument not last");
    }

    Definition d(arg);
    if (d.type & Type::NotRequired == Type::NotRequired)
    {
      if (haveMultiple)
      {
        throw std::logic_error("Invalid argument grammar: not required not allowed after multiple");
      }
        
      haveNotRequired = true;
    }
    else
    if (haveNotRequired)
    {
      throw std::logic_error("Invalid argument grammar: required argument after not required");      
    }
    else
    if (d.type & Type::Multiple == Type::Multiple)
    {
      haveMultiple = true;
    }

    def.emplace_back(d);
  }
}

Arguments ArgumentParser::Parse(const std::string& line)
{
  Arguments args;
  std::string::size_type pos1 = 0;
  for (const auto& type : types)
  {
    if (pos1 == line.length() || pos1 == std::string::npos)
    {
      if (type.second == Type::NotRequired) break;
      throw cmd::SyntaxError();
    }

    while (line[pos1] == ' ')
    {
      if (++pos1 == line.length())
      {
        throw cmd::SyntaxError();
      }
    }

    switch (type.second)
    {
      case Type::Grouped      :
      {
        if (line[pos1] == '"') // multiple arguments grouped
        {
          auto pos2 = line.find('"', pos1 + 1);
          if (pos2 == std::string::npos)
          {
            throw cmd::SyntaxError();
          }

          args.Push(type.first, line.substr(pos1 + 1, pos2 - pos1 - 1));
          pos1 = pos2 + 1;
          break;
        }
        // fall through for single ungrouped argument
      }
      case Type::NotRequired  :
      case Type::Single       :
      {
        auto pos2 = line.find(' ', pos1);
        if (pos2 == std::string::npos)
        {
          args.Push(type.first, line.substr(pos1));
          pos1 = std::string::npos;
        }
        else
        {
          args.Push(type.first, line.substr(pos1, pos2 - pos1));
          pos1 = ++pos2;
        }
        break;
      }
      case Type::Multiple     :
      {
        auto pos2 = line.length() - 1;
        while (line[pos2] == ' ') --pos2;
        args.Push(type.first, line.substr(pos1, pos2 - pos1 + 1));
        pos1 = std::string::npos;
        break;
      }
    }
  }

  if (pos1 < line.length() && pos1 != std::string::npos)
  {
    throw cmd::SyntaxError();
  }

  return args;
}

std::string ExtractCommand(const std::string& line)
{
  auto pos = line.find(' ');
  if (pos == 0) throw cmd::SyntaxError();
  return line.substr(0, pos);
}

} /* cmd namespace */

#ifdef TEST

#include <cassert>

int main()
{
  cmd::ArgumentParser ap("one:s two:s three:g four:n multi:m ");
  
  auto args = ap("one two three four");
  
  assert(args.Has("four"));
}

#endif
