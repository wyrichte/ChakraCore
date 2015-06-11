function f(x) 
{
  if (x < 0) {
    Debug.sourceDebugBreak();
  } else {
    f(x-1);
  }
}

f(1);
