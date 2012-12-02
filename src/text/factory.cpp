#include "text/factory.hpp"
#include "text/error.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "fs/diriterator.hpp"
#include "fs/path.hpp"
#include "logs/logs.hpp"

namespace text
{
Factory Factory::instance;

util::Error Factory::Initalize()
{
  fs::Path datapath = cfg::Get().Datapath() + fs::Path("text");
  
  
  try
  {
    fs::DirIterator it(datapath);
    fs::DirIterator end;
   
    // let's get all error's in one go 
    int errors = 0;

    for (; it != end; ++it)
    {
      if ((*it)[0] == '.') continue;
      fs::Path file(datapath + *it);

      std::vector<std::string> args;
      boost::split(args, *it, boost::is_any_of("."));
      std::string& name = args[0]; 
      boost::trim(name);
      boost::to_lower(name);

      try
      {
        TemplateParser templ(file.ToString());
        instance.templates.emplace(std::make_pair(name, templ.Create()));
      }
      catch (const text::TemplateError& e)
      {
        logs::error << "Template Initalize error (" + *it + "): " + e.Message() << logs::endl;
        ++errors;
      }
    }
    if (errors > 0)
    {
      std::ostringstream os;
      os << errors << " template errors. Please review errors above.";
      throw text::TemplateError(os.str());
    }
  }
  catch (const util::SystemError& e)
  {
    logs::error << e.Message() << logs::endl;
    return util::Error::Failure(e.Message());
  }
  return util::Error::Success();
}

Template Factory::GetTemplate(const std::string& templ)
{
  std::string name = boost::to_lower_copy(templ);
  if (instance.templates.find(name) == instance.templates.end())
    throw TemplateError("No such template (" + templ + ")");
  return instance.templates.at(name);
}
  
// end
}
