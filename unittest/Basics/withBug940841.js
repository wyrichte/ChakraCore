//reduced flags:  -MaxinterpretCount:1 -MaxSimpleJITRunCount:1
var i = 0;
while (i < 3) 
{
  (function () 
  {
    with ({}) 
    {
      __proto__;
    }
  })();
  i++;
}
WScript.Echo("PASS");
