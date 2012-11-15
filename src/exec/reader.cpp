#include "exec/reader.hpp"
#include "util/processreader.hpp"

namespace exec
{

Reader::Reader(util::ProcessReader& child, const std::string& file,
    std::vector<std::string> argv) :
  child(child), open(false)
{
  argv.insert(argv.begin(), file);
  util::ProcessReader::ArgvType env;
  child.Open(argv[0], argv, env);
  open = true;
}

Reader::~Reader()
{
  try
  {
    Close();
  }
  catch (const util::SystemError& e)
  {
    
  }
}

bool Reader::Getline(std::string& line)
{
  return child.Getline(line);
}

void Reader::Close()
{
  if (open)
  {
    if (child.Close(util::TimePair(1, 0))) return;
    std::cout << "SIGTERM" << std::endl;
    if (child.Kill(util::TimePair(1, 0))) return;
    std::cout << "SIGKILL" << std::endl;
    child.Kill(SIGKILL, util::TimePair(1, 0));
    open = false;
  }
}

} /* exec namespace */
