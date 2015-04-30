template<class T, int N> class A
{
  public:
  template<int N2> A<T,N> operator=(const A<T,N2>& z)
    {
    return A<T,N>();
    }
};

int main()
{
  A<double,4> x;
  A<double,7> y;
  x = y;
  return 0;
}
