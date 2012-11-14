#include <fstream>
#include <memory>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "text/template.hpp"
#include "text/error.hpp"
#include "logs/logs.hpp"

namespace text
{

Template::Template(const std::string& file) : 
  file(file),
  head(SectionType::Head),
  body(SectionType::Body),
  foot(SectionType::Foot)
{
  std::string line;
  std::ifstream io(file.c_str());
  if (!io) throw TemplateError("Unable to open template file: " + file); 
  
  std::ostringstream os;
  std::ostringstream var;
  std::ostringstream buf;

  bool open = false;
  bool close = false;
  bool read = false;
  bool first = false;
  bool logic = false;
  int skip = 0;

  TemplateSection* templ = &head;
  
  for (int i = 0, line = 0; io.good(); ++i)
  {
    char c;
    io >> std::noskipws >> c;


    // check if we have to skip certainc haracters (when closing logic)
    if (skip > 0)
    {
      --skip;
      if (c == '\n' || c == '\r' || c == '}' || c == ' ')
        continue;
      skip = 0;
    }

    // handle logic 
    if (logic)
    {
      if (c == '%')
      {
        std::string command = buf.str();
        boost::trim(command);
        boost::to_lower(command);

        // check if it's the start of a block
        if (command == "endblock")
        {
          // handle end of block
          logic = false;
          templ->RegisterBuffer(os.str());
        }
        else if (command == "body")
          templ = &body;
        else if (command == "foot")
          templ = &foot;
        else if (command != "head")
        {
          throw TemplateMalform(++line, ++i, command 
            + " is not proper logic. Must be {% [end]<head/body/foot> %}");
        }

        skip = 3;
        logic = false;

        // reset os for block
        os.str(std::string());

        continue;
      }
      
      buf << c;
      continue;
    }
  
    // line & char count for error
    if (c == '\n' || c == '\r')
    {
      i = 0;
      ++line;;
    }

    // read buffer
    if (read)
    {

      if (c == ' ') continue;
      if (c == '{') throw TemplateMalform(++line, ++i);
      if (!close && c == '}')
      {
        close = true;
        continue;
      }
      else if (close && c == '}')
      {
        read = false;
        open = false;
        close = false;
        first = false;
        templ->RegisterTag(var.str());
        os << "{{" << buf.str() << "}}";
        continue;
      }
      else if (!first && c == '|') first = true;
      if (!first) buf << static_cast<char>(std::tolower(c));
      var << c;
      continue;
    }

    if (!open && c == '{')  
    {
      open = true;
      continue;
    }
    else if (open && c == '%')
    {
      // got logic
      open = false;
      logic = true;
      buf.str(std::string());
      continue;   
    }
    else if (open && c != '{')
    {
      os << "{";
      open = false;
    }
    else if (open && c == '{') 
    {
      var.str(std::string());
      buf.str(std::string());
      read = true;
      continue;
    }
    os << c;
  }

  //buffer = os.str();
  // strip last char if it's new line
  //if (*buffer.rbegin() == '\n' || *buffer.rbegin() == '\r')
  //  buffer.erase(buffer.end()-1, buffer.end());
}

void TemplateSection::RegisterTag(std::string var)
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

void TemplateSection::CheckValueExists(const std::string& key)
{
  int i = 0;
  for (auto& value: values)
    if (key == value) ++i;
  if (i > 0)
    throw TemplateDuplicateValue("Already registered " + key);
}
  
void TemplateSection::RegisterValue(const std::string& key, const std::string& value)
{
  CheckValueExists(key);
  
  
  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.Parse(value);
  }

  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}

void TemplateSection::RegisterSize(const std::string& key, long long bytes)
{
  CheckValueExists(key);

  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.ParseSize(bytes);
  }

  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}

void TemplateSection::RegisterSpeed(const std::string& key, long long bytes, 
  long long xfertime)
{
  CheckValueExists(key);

  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.ParseSpeed(bytes, xfertime);
  }

  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}

std::string TemplateSection::Compile()
{
  std::string ret = buffer;
  for (auto& tag: tags)
  {
    boost::replace_first(ret, "{{" + tag.Name() + "}}", tag.Value());
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
    text::TemplateSection& head = temp.Head();
    head.RegisterValue("user", "io");
    head.RegisterValue("group", "some group");
    logs::debug << head.Compile() << logs::endl;
    text::TemplateSection& body = temp.Body();
    body.RegisterSize("amount", 15000000);
    body.RegisterSpeed("speed", 15000000, 666);
    logs::debug << body.Compile() << logs::endl;
    //temp.RegisterValue("b", "this is b");
    //logs::debug << temp.Compile() << logs::endl;
  }
  catch (const text::TemplateError& e)
  {
    logs::error << e.Message() << logs::endl;
  }
  return 0;
}
#endif

