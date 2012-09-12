#ifndef __NET_IP_HPP
#define __NET_IP_HPP

#include <stdexcept>
#include <string>

namespace util
{
  namespace ip
  {
    namespace versions
    {

      enum version
      {
        v4,
        v6,
        invalid
      };

    }

    typedef versions::version version_t;

    bool expand_v6(std::string& ip);
    bool is_valid(const std::string& ip);
    bool is_valid_v4(const std::string& ip);
    bool is_valid_v6(const std::string& ip);
    version_t parse_version(const std::string& ip);

  }
}

#endif
