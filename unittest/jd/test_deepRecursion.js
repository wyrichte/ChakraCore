function foo(n)
{
  if (n == 1) return Debug.sourceDebugBreak();
  return foo(n - 1) * n;
}
foo(50);
