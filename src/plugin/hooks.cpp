#include "plugin/hooks.hpp"

namespace plugin
{

boost::thread_specific_ptr<Hooks> Hooks::instance;


} /* script namespace */