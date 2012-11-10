#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "text/template.hpp"
#include "text/error.hpp"

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

  buffer = os.str();
  // strip last char if it's new line
  if (*buffer.rbegin() == '\n' || *buffer.rbegin() == '\r')
    buffer.erase(buffer.end()-1, buffer.end());
}

void Template::RegisterTag(std::string var)
{
  boost::trim(var);
  boost::to_lower(var);

  std::vector<std::string> args;
  boost::split(args, var, boost::is_any_of("|"));

  std::string name = args.front();
  args.erase(args.begin()); 
  Tag tag(name);;

  while (!args.empty())
  {
    auto filter = args.back();
    args.pop_back();
    
    tag.Register(filter);
  }

  tag.Compile();

  tags.emplace_back(tag);
 
}

void Template::CheckValueExists(const std::string& key)
{
  int i = 0;
  for (auto& value: values)
    if (key == value) ++i;
  if (i > 0)
    throw TemplateDuplicateValue("Already registered " + key);
}
  
void Template::RegisterValue(const std::string& key, const std::string& value)
{
  CheckValueExists(key);
  
  bool ok = false;
  for (auto& tag: tags)
    if (tag.Name() == key) 
    {
      ok = true;
      tag.Parse(value);
    }

  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}

void Template::RegisterSize(const std::string& key, long long bytes)
{
  CheckValueExists(key);

  bool ok = false;
  for (auto& tag: tags)
  {
    ok = true;
    tag.ParseSize(bytes);
  }

  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}

void Template::RegisterSpeed(const std::string& key, long long bytes, 
  long long xfertime)
{
  CheckValueExists(key);

  bool ok = false;
  for (auto& tag: tags)
  {
    ok = true;
    tag.ParseSpeed(bytes, xfertime);
  }

  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}

std::string Template::Compile()
{
  std::string ret = buffer;
  for (auto& tag: tags)
  {
    boost::replace_first(ret, "{{" + tag.Name() + "}}", tag.Value());
    //boost::replace_all(ret, "{{" + value.first + "}}", value.);
  }
  return ret;
}

}

#ifdef TEXT_TEMPLATE_TEST
#include "logs/logs.hpp"

int main()
{
  try
  {
    text::Template temp("data/text/test.tmpl");
    temp.RegisterSize("a", 140509184);
    temp.RegisterValue("b", "this is b");
    logs::debug << temp.Compile() << logs::endl;
  }
  catch (const text::TemplateError& e)
  {
    logs::error << e.Message() << logs::endl;
  }
  return 0;
}
#endif

