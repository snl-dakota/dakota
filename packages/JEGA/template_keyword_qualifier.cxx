class X
{
  public:
  template<int> void member() {}
  template<int> static void static_member() {}
};
template<class T> void f(T* p)
{
  p->template member<200>(); // OK: < starts template argument
  T::template static_member<100>(); // OK: < starts explicit qualification
}

int main()
{
  X foo = X();
  return 0;
}
