function print(x)
{
    WScript.Echo(x);
}

var numTests = 2;
var testPassed = 0;

// In this case, arguments refers to the arguments object of the parent function and so it 
// needs to be defined. See BLUE: 521703
try
{
  (function() {
    function arguments::hello()
    {
    }
  })();
}
catch (ex)
{
}

testPassed++;

try
{
    new Function("function arguments::functional(c){}")();
}
catch(ex)
{
}

testPassed++;

if (testPassed == numTests) {
    print("PASS");
}