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
#include "util/path/diriterator.hpp"
#include "version.hpp"
#include "db/error.hpp"
#include "db/connection.hpp"

std::shared_ptr<cfg::Config> config;

void DisplayHelp(char* argv0, boost::program_options::options_description& desc)
{
  std::cout << "usage: " << argv0 << " [options] [user][:[group]] <path> [<path>..]" << std::endl;
  std::cout << desc;
}

void DisplayVersion()
{
  std::cout << "ebftpd chown " + std::string(version) << std::endl;
}

bool ParseOptions(int argc, char** argv, bool& recursive, std::string& configPath,
                  std::string& user, std::string& group, std::vector<std::string>& paths)
{
  namespace po = boost::program_options;
  po::options_description visible("supported options");
  visible.add_options()
    ("help,h", "display this help message")
    ("version,v", "display version")
    ("config-path,c", po::value<std::string>(), "specify location of config file")
    ("recursive,R", "apply changes recursively")
  ;

  std::string who;
  po::options_description all("positional options");
  all.add(visible);
  all.add_options()
    ("who", po::value<std::string>(&who)->required(), "who")
    ("paths", po::value(&paths)->required(), "paths")
  ;

  po::positional_options_description pos;
  pos.add("who", 1);
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

    boost::smatch match;
    if (!boost::regex_match(who, match, boost::regex("(\\w+)?(?::(\\w+))?")))
      throw boost::program_options::error("invalid user:group option specified");

    user = match[1].str();
    group = match[2].str();

    if (user.empty() && group.empty())
      throw boost::program_options::error("invalid user:group option specified");
    
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << std::endl;
    DisplayHelp(argv[0], visible);
    return false;
  }

  if (vm.count("config-path")) configPath = vm["config-path"].as<std::string>();
  recursive = vm.count("recursive") > 0;
  return true;
}

template <typename Iterator>
void SetOwner(Iterator begin, Iterator end, const fs::Owner& owner, bool recursive)
{
  using namespace util::path;

  for (auto it = begin; it != end; ++it)
  {
    const std::string& path = *it;
    
    auto e = fs::SetOwner(path, owner);
    if (!e) std::cerr << path << ": " << e.Message() << std::endl;
    else
    if (recursive)
    {
      try
      {
        auto status = Status(path);
        if (status.IsDirectory() && !status.IsSymLink())
        {
          SetOwner(DirIterator(path, DirIterator::AbsolutePath), 
                   DirIterator(), owner, recursive);
        }
      }
      catch (const util::SystemError& e)
      {
        std::cerr << path << ": " << e.Message() << std::endl;
      }
    }
  }
}

acl::UserID LookupUID(db::SafeConnection& conn, const std::string& user)
{
  auto query = QUERY("name" << user);
  auto cursor = conn.BaseConn().query(config->Database().Name() + ".users", query, 1);
  db::LastErrorToException(conn.BaseConn());
  if (!cursor.get()) throw mongo::DBException("Connection failure", 0);
  if (!cursor->more()) throw util::RuntimeError("User doesn't exist: " + user);
  return cursor->next()["uid"].Int();
}

acl::GroupID LookupGID(db::SafeConnection& conn, const std::string& group)
{
  auto query = QUERY("name" << group);
  auto cursor = conn.BaseConn().query(config->Database().Name() + ".groups", query, 1);
  db::LastErrorToException(conn.BaseConn());
  if (!cursor.get()) throw mongo::DBException("Connection failure", 0);
  if (!cursor->more()) throw util::RuntimeError("Group doesn't exist: " + group);
  return cursor->next()["gid"].Int();
}

util::Error LookupOwner(const std::string& user, const std::string& group, fs::Owner& owner)
{
  auto dbConfig = config->Database();  
  try
  {
    db::SafeConnection conn;
    owner = fs::Owner(user.empty() ? -1 : LookupUID(conn, user),
                      group.empty() ? -1 : LookupGID(conn, group));
  }
  catch (const mongo::DBException& e)
  {
    return util::Error::Failure(e.what());
  }
  catch (const util::RuntimeError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();
}

int main(int argc, char** argv)
{
  std::string user;
  std::string group;
  std::vector<std::string> paths;
  std::string configPath;
  bool recursive = false;

  if (!ParseOptions(argc, argv, recursive, configPath, user, group, paths)) return 1;

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

  fs::Owner owner(-1, -1);
  auto e = LookupOwner(user, group, owner);
  if (!e)
  {
    std::cerr << "Unable to look up owner in database: " << e.Message() << std::endl;
    _exit(1);
  }

  SetOwner(paths.begin(), paths.end(), owner, recursive);
  _exit(0); // used to work around a mongodb driver bug
}
