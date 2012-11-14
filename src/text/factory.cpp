#include <boost/algorithm/string.hpp>
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

    for (; it != end; ++it)
    {
      if ((*it)[0] == '.') continue;
      fs::Path file(datapath + *it);

      logs::debug << file.ToString() << logs::endl;

      std::vector<std::string> args;
      boost::split(args, *it, boost::is_any_of("."));
      std::string& name = args[0]; 
      boost::trim(name);
      boost::to_lower(name);

      try
      {
        Template templ(file.ToString());
        templ.Initalize();
        // emplace our template
        instance.templates.emplace(std::make_pair(name, templ));
      }
      catch (const text::TemplateError& e)
      {
        logs::error << "Template Initalize error (" + *it + "): " + e.Message() << logs::endl;
      }
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
