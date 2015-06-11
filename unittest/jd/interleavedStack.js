
var count = 0;
function foo()
{
  bar(); /**bp:stack()**/
}
function bar()
{
    if(count++ < 3)
	{
		Debug.invokeFunction(foo); 
	}
}
bar();

  WScript.Echo("PASSED");  