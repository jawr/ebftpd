#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "text/template.hpp"
#include "text/error.hpp"
#include "logs/logs.hpp"

namespace text
{

Template::Template(const std::string& file) : file(file)
{
  std::string line;
  std::ifstream io(file.c_str());
  if (!io) throw TemplateError("Unable to open template file: " + file); 
  
  std::ostringstream os;
  std::ostringstream var;

  bool open = false;
  bool read = false;
  bool first = false;
  
  for (int i = 0, line = 0; io.good(); ++i)
  {
    char c;
    io >> std::noskipws >> c;

    if (c == '\n' || c == '\r')
    {
      i = 0;
      ++line;;
    }

    if (read)
    {
      if (c == '{') throw TemplateMalform(++line, ++i);
      if (c == '}')
      {
        read = false;
        open = false;
        first = false;
        Register(var.str());
        os << " " << c;
        continue;
      }
      else if (!first && c == '|') first = true;
      if (!first) os << c;
      var << c;
      continue;
    }

    if (!open && c == '{') open = true;
    else if (open && c != '{') open = false;
    else if (open && c == '{') 
    {
      var.str(std::string());
      read = true;
    }

    os << c;
  }

  logs::debug << "OS:" << logs::endl;
  logs::debug << os.str() << logs::endl;
}

void Template::Register(std::string var)
{
  boost::trim(var);
  boost::to_lower(var);

  logs::debug << "Register cookie: " << var << logs::endl;

  std::vector<std::string> args;
  boost::split(args, var, boost::is_any_of("|"));

  std::string name = args.front();
  args.erase(args.begin()); 
  Type type;

  while (!args.empty())
  {
    auto filter = args.back();
    args.pop_back();
    
    type.Register(filter);
  }

  type.Compile();
  logs::debug << "Type::Format: " << type.Format() << logs::endl;
}

}

#ifdef TEXT_TEMPLATE_TEST
int main()
{
  try
  {
    text::Template temp("data/text/test.tmpl");
  }
  catch (const text::TemplateError& e)
  {
    logs::error << e.Message() << logs::endl;
  }
  return 0;
}
#endif

