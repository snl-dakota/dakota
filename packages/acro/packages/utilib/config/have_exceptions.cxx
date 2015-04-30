int main ()
{
  try
    {
    throw 1;
    }
  catch (int i)
    {
    return i;
    }
  return 0;
}
