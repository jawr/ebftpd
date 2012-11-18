#include <stdexcept>
#include "fs/path.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"
#include "fs/directory.hpp"

namespace fs
{

Path MakeRelative(const VirtualPath& path)
{
  return Path(util::path::Relative(WorkDirectory().ToString(), path.ToString()));
}

Path MakePretty(const VirtualPath& path)
{
  Path pretty(MakeRelative(path));
  if (pretty.IsEmpty()) return Path(".");
  if (pretty.ToString().compare(0, 3, "../")) return pretty;
  return Path(path.ToString());
}

Path MakePretty(const Path& path)
{
  if (path.IsAbsolute()) return path;
  return MakePretty(MakeVirtual(path));
}

const VirtualPath& MakeVirtual(const Path& path)
{
  if (!path.cache.virt)
    path.cache.virt = new VirtualPath(WorkDirectory() / path);
  return *path.cache.virt;
}

const VirtualPath& MakeVirtual(const VirtualPath& path)
{
  return path;
}

const VirtualPath& MakeVirtual(const RealPath& path)
{
  if (!path.cache.virt)
  {
    const std::string& sitepath = cfg::Get().Sitepath().ToString();
    const std::string& pathstr = path.ToString();
    if (!pathstr.compare(0, sitepath.length(), sitepath) &&
        (pathstr[sitepath.length()] == '/' ||
         pathstr.length() == sitepath.length()))
    {
      std::string newpath(path.ToString(), cfg::Get().Sitepath().Length());
      if (newpath.empty()) newpath = "/";
      path.cache.virt = new VirtualPath(newpath);
    }
    else
      throw std::logic_error("RealPath cannot be made VirtualPath");
  }
  return *path.cache.virt;
}

const RealPath& MakeReal(const Path& path)
{
  if (!path.cache.real)
    path.cache.real = new RealPath(RealPath(cfg::Get().Sitepath()) & MakeVirtual(path));
  return *path.cache.real;
}

const RealPath& MakeReal(const VirtualPath& path)
{
  if (!path.cache.real)
    path.cache.real = new RealPath(RealPath(cfg::Get().Sitepath()) & path);
  return *path.cache.real;
}

const RealPath& MakeReal(const RealPath& path)
{
  return path;
}

} /* fs namespace */

#ifdef FS_PATH_TEST

#include <iostream>
#include <typeinfo>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace fs;

void SomePathFunc(const Path&) { }
void SomeAbsFunc(const VirtualPath&) { }
void SomeRealFunc(const RealPath&) { }

int main()
{
  Path p("/hello/world.txt");
  
  std::cout << "Path testing\n" << std::endl;
  
  std::cout << "path: " << p << std::endl;
  std::cout << "dirname: " << p.Dirname() << std::endl;
  std::cout << "basename: " << p.Basename() << std::endl;
  std::cout << "extension: " << p.Extension() << std::endl;
  std::cout << "noextension: " << p.NoExtension() << std::endl;
  std::cout << "makevirtual: " << MakeVirtual(p) << std::endl;
  std::cout << "makereal: " << MakeReal(p) << std::endl;
  std::cout << "truecmp: " << (p == p) << std::endl;
  std::cout << "falsecmp: " << (p == MakeVirtual(Path("xcx"))) << std::endl;
  std::cout << "truecmp: " << (p == p.ToString()) << std::endl;
  std::cout << "falsecmp: " << (p == Path("xcx").ToString()) << std::endl;
  std::cout << "truecmp: " << (p == p.CString()) << std::endl;
  std::cout << "falsecmp: " << (p == Path("xcx").CString()) << std::endl;
  std::cout << "truegreater: " << (Path("z") > Path("a")) << std::endl;
  std::cout << "falsegreater: " << (Path("a") > Path("a")) << std::endl;
  std::cout << "trueless: " << (Path("a") < Path("z")) << std::endl;
  std::cout << "falseless: " << (Path("z") < Path("a")) << std::endl;
  
  std::cout << "\nVirtual testing\n" << std::endl;
  
  VirtualPath v(MakeVirtual(p));
  
  std::cout << "path: " << v << std::endl;
  std::cout << "dirname: " << v.Dirname() << std::endl;
  std::cout << "basename: " << v.Basename() << std::endl;
  std::cout << "extension: " << v.Extension() << std::endl;
  std::cout << "noextension: " << v.NoExtension() << std::endl;
  std::cout << "makevirtual: " << MakeVirtual(v) << std::endl;
  std::cout << "makereal: " << MakeReal(v) << std::endl;
  std::cout << "makerelative: " << MakeRelative(v) << std::endl;
  std::cout << "frmorelative: " << Resolve(MakeVirtual(MakeRelative(v))) << std::endl;
  std::cout << "makepretty: " << MakePretty(v) << std::endl;
      
  std::cout << "\nReal testing\n" << std::endl;
  
  RealPath r(MakeReal(p));
  
  std::cout << "path: " << r << std::endl;
  std::cout << "dirname: " << r.Dirname() << std::endl;
  std::cout << "basename: " << r.Basename() << std::endl;
  std::cout << "extension: " << r.Extension() << std::endl;
  std::cout << "noextension: " << r.NoExtension() << std::endl;
  std::cout << "makevirtual: " << MakeVirtual(r) << std::endl;
  std::cout << "makereal: " << MakeReal(r) << std::endl;

  boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();  
  
  for (int i = 0; i < 100000; ++i)
  {
    //p = Resolve(p);
    MakeVirtual(p);
    MakeReal(p);
    MakeVirtual(p).Dirname();
    MakeReal(p).Basename();
  }

  {
    Path p("hello");
    VirtualPath v(MakeVirtual("world"));
    
    std::cout << p << std::endl;
    std::cout << v << std::endl;
    std::cout << v / p << std::endl;
    std::cout << p / v << std::endl;
    std::cout << (p & v) << std::endl;
  }
  
  {
    Path p = "/tmp";
    
    VirtualPath vp(MakeVirtual(p));
    VirtualPath vm(std::move(vp));
    std::cout << p << std::endl;
  }
  
  std::cout << (boost::posix_time::microsec_clock::local_time() - start) << std::endl;
}

#endif