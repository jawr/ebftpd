#include <iostream>
#include <functional>

template <typename R, typename... Args>
struct Wrapper
{
  typedef R(*Function)(Args...);
  Function func;

  Wrapper(Function func) : func(func) { }

  R operator()(Args&&... args)
  {
    std::cout << "before" << std::endl;
    func(std::forward<Args>(args)...);
    std::cout << "after" << std::endl;
  }
};

template <typename R, typename... Args>
std::function<R(Args...)> MakeWrapper(R(*function)(Args...))
{
  return Wrapper<R, Args...>(function);
}

struct SomeClass
{
  void MemberFunc() { }
  void MemberFunc() const { }
};

int main()
{
  std::function<void(const SomeClass*) const> f = &SomeClass::MemberFunc;

  const SomeClass sc;

  f(&sc);
}
