//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "text/factory.hpp"
#include "text/error.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "fs/diriterator.hpp"
#include "fs/path.hpp"
#include "logs/logs.hpp"

namespace text
{
std::mutex Factory::mutex;
std::unique_ptr<Factory> Factory::instance;

util::Error Factory::Initialize()
{
  std::unique_ptr<Factory> factory(new Factory());
  
  auto textpath = fs::Path(cfg::Get().Datapath()) / "text";
  
  try
  {
    util::path::DirIterator it(textpath.ToString());
    util::path::DirIterator end;
   
    // let's get all error's in one go 
    int errors = 0;

    for (; it != end; ++it)
    {
      fs::Path p(*it);
      if (p.Extension() != "tmpl") continue;
      
      fs::Path file(textpath / p);
      std::string name = p.NoExtension(); 
      util::ToLower(name);

      try
      {
        TemplateParser templ(file.ToString());
        factory->templates.insert(std::make_pair(name, templ.Create()));
      }
      catch (const text::TemplateError& e)
      {
        logs::Error("Template Initialize error (%1%): %2%", *it, e.Message());
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
    logs::Error("Unable to open template directory: %1%", e.Message());
    return util::Error::Failure(e.Message());
  }

  std::lock_guard<std::mutex> lock(mutex);
  instance = std::move(factory);
  return util::Error::Success();
}

Template Factory::GetTemplate(const std::string& templ)
{
  std::string name = util::ToLowerCopy(templ);
  
  std::lock_guard<std::mutex> lock(mutex);
  if (!instance.get() || instance->templates.find(name) == instance->templates.end())
    throw TemplateError("No such template (" + templ + ")");
  return instance->templates.at(name);
}
  
// end
}
