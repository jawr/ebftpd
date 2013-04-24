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

#ifndef __TEXT_FACTORY_HPP
#define __TEXT_FACTORY_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "text/template.hpp"
#include "text/parser.hpp"
#include "text/templatesection.hpp"
#include "util/error.hpp"

namespace text
{

class Factory
{
  std::unordered_map<std::string, Template> templates;
  
  static std::mutex mutex;
  static std::unique_ptr<Factory> instance;

  Factory() = default;

public:
  
  static util::Error Initialize();
  static int Size()
  {
    std::lock_guard<std::mutex> lock(mutex);
    return instance->templates.size();
  }
  static Template GetTemplate(const std::string& templ);
};
  
// end
}
#endif
