#include <string>

template <typename T1, typename T2>
struct Test
{
  T1 t1;
  T2 t2;

};

template <typename T2>
using TestString = Test<std::string, T2>;

int main()
{
}

