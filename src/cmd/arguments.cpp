#include "cmd/arguments.hpp"

namespace cmd
{

void ArgumentParser::Compile(const std::string& grammar)
{
  std::vector<std::string> args;
  util::Split(args, grammar, " ");
  bool haveMultiple = false;
  for (std::string& arg : args)
  {
    if (haveMultiple) // multiple must be last argument
    {
      throw std::logic_error("Invalid argument grammar 1");
    }

    std::vector<std::string> toks;
    util::Split(toks, arg, ":");
    if (toks.size() != 2) throw std::logic_error("Invalid argument grammar 2");

    if (toks[1] == "single" || toks[1] == "s")
    {
      types.emplace_back(toks[0], Type::Single);
    }
    else if (toks[1] == "grouped" || toks[1] == "g")
    {
      types.emplace_back(toks[0], Type::Grouped);
    }
    else if (toks[1] == "multiple" || toks[1] == "m")
    {
      haveMultiple = true;
      types.emplace_back(toks[0], Type::Multiple);
    }
    else throw std::logic_error("Invalid argument grammar 3");
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
      case Type::Grouped  :
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
      case Type::Single   :
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
      case Type::Multiple :
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
