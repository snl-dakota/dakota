namespace Outer
{
  namespace Inner
  {
    int i = 0;
  }
}

using namespace Outer::Inner;
int main()
{
  return i;
}
