var a = new Array(2000000);
var i = 0;
try
{
   while (true)
   {
       a[a.length] = new Object();
   }
}
catch (e)
{
}

for (var i = 0; i < 10; i++)
{
    WScript.Echo(i);
    CollectGarbage();
}
