#include <cstdio>
#include <cstdlib>
#include "util/verify.hpp"

extern char *__progname;

void __verify_fail(const char *assertion, const char *file,
                   unsigned int line,const char *function)
{
  fprintf(stderr,"%s%s%s:%u: %s%sAssertion `%s' failed.\n",
          __progname, __progname[0] ? ": " : "",
          file, line, function ? function : "",
          function ? ": " : "", assertion);
  fflush(stdout);
  abort();
}
