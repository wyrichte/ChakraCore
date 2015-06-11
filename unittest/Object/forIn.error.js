// WinBlue 194710: wrong line/col number in exception from inside for in.

function foo()
{
    for (var p in c) 	// Error: c is undefined.
    {
        addPropertyName(p);
    }
}

try
{
  foo();
}
catch (ex)
{
  var truncatedPathStack = ex.stack.replace(/\(.*\\/g, "(");
  WScript.Echo(truncatedPathStack);
}
