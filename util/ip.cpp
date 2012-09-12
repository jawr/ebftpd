#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ip.hpp"

namespace util
{
  namespace ip
  {

    bool expand_v6(std::string& ip)
    {
      int colon_count = 0;
      std::string::size_type pos;

      for (pos = 0; pos < ip.length(); ++pos)
      {
        if (ip[pos] == ':')
        {
          ++colon_count;
        }
      }

      if (colon_count < 7)
      {
        pos = ip.find("::");

        if (pos == std::string::npos)
        {
          return false;
        }

        ip.insert(ip.begin() + pos + 1, 7 - colon_count, ':');
      }

      while ((pos = ip.find("::")) != std::string::npos)
      {
        ip.insert(ip.begin() + pos + 1, 1, '0');
      }

      return true;
    }

    bool is_valid(const std::string& ip)
    {
      return is_valid_v4(ip) ||
             is_valid_v6(ip);
    }

    bool is_valid_v4(const std::string& ip)
    {
      struct sockaddr_in addr;
      return inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) == 1;
    }

    bool is_valid_v6(const std::string& ip)
    {
      struct sockaddr_in6 addr;
      return inet_pton(AF_INET6, ip.c_str(), &addr.sin6_addr) == 1;
    }

    version_t parse_version(const std::string& ip)
    {
      if (is_valid_v4(ip))
      {
        return versions::v4;
      }
      else if (is_valid_v6(ip))
      {
        return versions::v6;
      }

      return versions::invalid;
    }

  }
}
