#ifndef __SCRIPTING_PYTHON_PYTHON_HPP

namespace scripting { namespace python
{

struct PythonReleaseGIL
{
  static void Unlock() { /* unlock GIL */ }
  static void Lock() { /* lock GIL */ }
  
  PythonReleaseGIL()
  {
    Unlock();
  }
  
  ~PythonReleaseGIL()
  {
    Lock();
  }
};



} /* python namespace */
} /* scripting namespace */

#endif
