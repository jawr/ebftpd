#ifndef __VERIFY_HPP
#define __VERIFY_HPP

#define verify(expr) \
  ((expr) \
   ? static_cast<void> (0) \
   : __verify_fail (__STRING(expr), __FILE__, __LINE__, __PRETTY_FUNCTION__))

void __verify_fail(const char *assertion, const char *file,
                   unsigned int line,const char *function);

#endif
