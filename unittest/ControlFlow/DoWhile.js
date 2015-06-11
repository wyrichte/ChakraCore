// Following two cases are supported only in ES5

// Use do..while as a statement inside if..else
var str="if (1) do WScript.Echo(1); while (false); else 1;";

try
{
	eval(str);
}
catch (e)
{
	WScript.Echo(e);
}

// Use do..while as a statement inside another do..while
str="do do WScript.Echo(2); while (false); while(false);"
try
{
	eval(str);
}
catch (e)
{
	WScript.Echo(e);
}

// All the following cases should succeed in pre-ES5 and ES5

// do..while withuot a semicolon at the end, followed by another statement
// do while surrounds a statement without a semicolon, but ended with a newline
var a = 10;
do
  WScript.Echo(3)
while (false)
var b=20;

with(a) do WScript.Echo(4); while (false)

for(var i=0; i<5; i++)
  do
    WScript.Echo("5."+i);
  while(false)

// do..while as the last statement ended by EOF
do
  WScript.Echo(6)
while (false)