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

#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/regex.hpp>
#include "cfg/get.hpp"
#include "cfg/error.hpp"
#include "fs/owner.hpp"
#include "util/path/status.hpp"
#include "util/path/globiterator.hpp"
#include "version.hpp"
#include "db/error.hpp"
#include "util/path/path.hpp"
#include "util/misc.hpp"
#include "util/enumbitwise.hpp"
#include "db/connection.hpp"

std::shared_ptr<cfg::Config> config;
std::unique_ptr<db::FastConnection> conn;

void DisplayHelp(char* argv0, boost::program_options::options_description& desc)
{
  std::cout << "usage: " << argv0 << " [options] [user][:[group]] [<path> [<path>..]]" << std::endl;
  std::cout << desc;
}

void DisplayVersion()
{
  std::cout << "ebftpd index " + std::string(version) << std::endl;
}

enum class Mode
{
  Both,
  AddOnly,
  DeleteOnly
};

bool ParseOptions(int argc, char** argv, Mode& mode, std::string& configPath, 
      std::vector<std::string>& paths)
{
  namespace po = boost::program_options;
  po::options_description visible("supported options");
  visible.add_options()
    ("help,h", "display this help message")
    ("version,v", "display version")
    ("config-path,c", po::value<std::string>(), "specify location of config file")
    ("add-only,a", "add new directories only")
    ("delete-only,d", "delete missing directories only")
  ;
  
  std::string who;
  po::options_description all("positional options");
  all.add(visible);
  all.add_options()("paths", po::value(&paths), "paths");

  po::positional_options_description pos;
  pos.add("paths", -1);

  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).options(all).
              positional(pos).run(), vm);

    if (vm.count("help"))
    {
      DisplayHelp(argv[0], visible);
      return false;
    }

    if (vm.count("version"))
    {
      DisplayVersion();
      return false;
    }

    po::notify(vm);

    if (vm.count("add-only") > 0 && vm.count("delete-only") > 0)
      throw boost::program_options::error("add-only and delete-only cannot be used together");
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << std::endl;
    DisplayHelp(argv[0], visible);
    return false;
  }

  if (vm.count("config-path")) configPath = vm["config-path"].as<std::string>();
  if (vm.count("add-only") > 0) mode = Mode::AddOnly;
  else if (vm.count("delete-only") > 0) mode = Mode::DeleteOnly;
  else mode = Mode::Both;
  return true;
}

std::string VirtualPath(std::string path)
{
  const std::string& sitepath = config->Sitepath();
  if (!path.compare(0, sitepath.length(), sitepath) &&
      (path[sitepath.length()] == '/' ||
       path.length() == sitepath.length()))
  {
    path.erase(0, sitepath.length());
    if (path.empty()) path = "/";
    else if (path.back() == '/') path.erase(path.end() - 1);
    return path;
  }
  return std::string();
}

void AddPath(const std::string& path)
{
  std::cout << path << std::endl;
  std::string collection = config->Database().Name() + ".index";
  util::path::GlobIterator globEnd;
  auto globIter = util::path::GlobIterator(path, path.back() != '/');
  for (; globIter != globEnd; ++globIter)
  {
    if (!util::path::IsDirectory(*globIter)) continue;
    auto vpath = VirtualPath(*globIter);
    if (!vpath.empty())
    {
      auto section = config->SectionMatch(vpath, true);
      auto obj = BSON("path" << vpath << "section" << (section ? section->Name() : ""));
      conn->BaseConn().insert(collection, obj);
    }
  }
}

void AddPaths(const std::vector<std::string>& paths)
{
  std::for_each(paths.begin(), paths.end(), AddPath);
}

void DeletePath(const std::string& path)
{
  std::string vpath = VirtualPath(path);
   if (vpath.empty()) return;
   
  std::vector<std::string> toDelete;
  std::string collection = config->Database().Name() + ".index";
  std::string regex = util::WildcardToRegex(vpath);
  mongo::BSONObjBuilder bob;
  bob.appendRegex("$regex", regex);
  auto query = QUERY("path" << bob.obj());
  auto cursor = conn->BaseConn().query(collection, query);
  while (cursor->more())
  {
    auto obj = cursor->next();
    std::string path = obj["path"].String();
    try
    {
      util::path::Status(util::path::Append(config->Sitepath(), path));
    }
    catch (const util::SystemError& e)
    {
      if (e.Errno() == ENOENT)
      {
        toDelete.emplace_back(path);
      }
    }
  }
  
  for (const std::string& path : toDelete)
  {
    auto query = QUERY("path" << path);
    conn->BaseConn().remove(collection, query);
  }
}

void DeletePaths(const std::vector<std::string>& paths)
{
  std::for_each(paths.begin(), paths.end(), DeletePath);
}

std::vector<std::string> ConfigPaths()
{
  std::vector<std::string> paths;
  for (const std::string& path : config->Indexed())
  {
    bool trailingSlash = path.back() == '/';
    paths.emplace_back(util::path::Append(config->Sitepath(), path));
    if (trailingSlash) paths.back() += '/';
  }
  return paths;
}

bool ValidatePaths(const std::vector<std::string>& paths)
{
  bool okay = true;
  for (const std::string& path : paths)
  {
    if (!config->IsIndexed(VirtualPath(path)))
    {
      okay = false;
      std::cerr << "Path is not set as indexed in config file: " << path << std::endl;
    }
  }
  return okay;
}

bool ConnectDatabase()
{
  try
  {
    conn.reset(new db::FastConnection());
    return true;
  }
  catch (const db::DBError& e)
  {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

int main(int argc, char** argv)
{
  std::vector<std::string> paths;
  std::string configPath;
  Mode mode = Mode::Both;

  if (!ParseOptions(argc, argv, mode, configPath, paths)) return 1;

  try
  {
    config = cfg::Config::Load(configPath, true);
    cfg::UpdateShared(config);
  }
  catch (const cfg::ConfigError& e)
  {
    std::cerr << "Failed to load config: " << e.Message() << std::endl;
    return 1;
  }
  
  
  if (config->Indexed().empty())
  {
    std::cerr << "No indexed paths set in config." << std::endl;
    return 1;
  }

  if (!paths.empty() && !ValidatePaths(paths)) return 1;
  if (!ConnectDatabase()) return 1;
  
  try
  {
    if (mode == Mode::Both || mode == Mode::AddOnly) 
    {
      if (paths.empty()) paths = ConfigPaths();
      AddPaths(paths);
    }
    
    if (mode == Mode::Both || mode == Mode::DeleteOnly) 
    {
      if (paths.empty()) paths.emplace_back(util::path::Append(config->Sitepath(), "*"));
      DeletePaths(paths);
    }
  }
  catch (const mongo::DBException& e)
  {
    std::cerr << "Error while communicating with database: " << e.what() << std::endl;
    _exit(1);
  }
  _exit(0); // used to work around a mongodb driver bug
}
