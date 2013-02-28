#include <string>
#include "script.hpp"
#include "ftp/client.hpp"
#include "fs/path.hpp"
#include "exec/reader.hpp"

namespace exec
{

int Script(ftp::Client& client, const util::ProcessReader::ArgvType& argv, std::string& messages)
{
  exec::Reader reader(client, argv);
  messages.clear();

  std::string line;
  while (reader.Getline(line))
  {
    if (!messages.empty()) messages += '\n';
    messages += line;
  }
  
  reader.Close();
  return reader.ExitStatus();
}

} /* exec namespace */
