#include <fstream>
#include <memory>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/error.hpp"
#include "logs/logs.hpp"

namespace text
{

void Template::Initalize()
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
          throw TemplateMalform(++line, ++i, "(" + command + ")"
            + " incorrect syntax. Must be {% endblock|head|body|foot %}");
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
}

}

#ifdef TEXT_TEMPLATE_TEST
#include "logs/logs.hpp"
#include "text/factory.hpp"
#include "cfg/get.hpp"
#include "cfg/config.hpp"
int main()
{
  cfg::UpdateShared(std::shared_ptr<cfg::Config>(new cfg::Config("ftpd.conf"))); 
  text::Factory::Initalize();
  logs::debug << "Templates loaded: " << text::Factory::Size() << logs::endl;

  try
  {
    text::Template groups = text::Factory::GetTemplate("groups");
    std::ostringstream os;
    text::TemplateSection& head = groups.Head();
    os << head.Compile();
    text::TemplateSection& body = groups.Body();
    for (int i = 0; i < 5; ++i)
    {
      body.Reset();
      body.RegisterValue("users", i);
      body.RegisterValue("group", i);
      body.RegisterValue("description", "HELLO THIS IS A DESCR");
      os << body.Compile();
    }
    text::TemplateSection& foot = groups.Foot();
    foot.RegisterValue("total_groups", 5);
    os << foot.Compile();
    logs::debug << "COMPILED:" << logs::endl;
    logs::debug << os.str() << logs::endl;
  }
  catch (const text::TemplateError& e)
  {
    logs::error << e.Message() << logs::endl;
    return 1;
  }
  
  return 0;
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

