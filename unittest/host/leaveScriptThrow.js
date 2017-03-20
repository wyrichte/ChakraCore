function __onLeaveScript()
{
  print('__onLeaveScript');
  throw new Error("2nd throw from __onLeaveScript");
}

throw new Error("1st throw");
