#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
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
      if (c == ' ') continue;
      if (c == '{') throw TemplateMalform(++line, ++i);
      if (c == '}')
      {
        read = false;
        open = false;
        first = false;
        RegisterTag(var.str());
        os << c;
        continue;
      }
      else if (!first && c == '|') first = true;
      if (!first) os << static_cast<char>(std::tolower(c));
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
  buffer = os.str();
  logs::debug << "Tags: " << tags.size() << logs::endl;
}

void Template::RegisterTag(std::string var)
{
  boost::trim(var);
  boost::to_lower(var);

  logs::debug << "Register cookie: " << var << logs::endl;

  std::vector<std::string> args;
  boost::split(args, var, boost::is_any_of("|"));

  std::string name = args.front();
  args.erase(args.begin()); 
  Tag tag;

  while (!args.empty())
  {
    auto filter = args.back();
    args.pop_back();
    
    tag.Register(filter);
  }

  tag.Compile();

  tags.emplace(std::make_pair(name, tag));
  logs::debug << "Type::Format: " << tag.Format() << logs::endl;
}

void Template::RegisterValue(const std::string& key, const std::string& value)
{
  auto it = tags.find(key);
  if (it == tags.end()) 
    throw TemplateNoTag("No template tag with key: " + key);

  Tag& tag = it->second;
  
  logs::debug << "Template::RegisterValue: " << key << " -> " 
    << value << logs::endl;

  std::ostringstream os;
  os << boost::format(tag.Format()) % value;

  logs::debug << "|" << os.str() << "|" << logs::endl;   

  if (values.find(key) != values.end())
    throw TemplateDuplicateValue("Already registered " + key);

  values.emplace(std::make_pair(key, os.str()));
}

std::string Template::Compile()
{
  std::string ret = buffer;
  for (auto value: values)
  {
    boost::replace_all(ret, "{{" + value.first + "}}", value.second);
  }
  return ret;
}

}

#ifdef TEXT_TEMPLATE_TEST
int main()
{
  try
  {
    text::Template temp("data/text/test.tmpl");
    temp.RegisterValue("hello", "Womwata");
    temp.RegisterValue("test", "Womwata");
    logs::debug << temp.Compile() << logs::endl;
  }
  catch (const text::TemplateError& e)
  {
    logs::error << e.Message() << logs::endl;
  }
  return 0;
}
#endif

