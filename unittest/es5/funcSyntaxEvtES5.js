

function write(a) {
    if (this.WScript == undefined) {
        document.write(a);
        document.write("</br>");
    }
    else
        WScript.Echo(a);
}

function Assert(expected, actual, name)
{
    if (expected != actual) {
        write("Assertion failed! Expected: " + expected + ", actual " + actual);
    }
}

function TestMethodThrowsException(method, args)
{
    if (!args) args = [];

    var exceptionThrown = false;
    try {
        eval(method).apply(null, args);
    } catch (ex) {
        write("Exception thrown: " + ex);
        exceptionThrown = true;
    }

    Assert(true, exceptionThrown, method);
}

try
{
// This should compile in ES5 but throw an error on binding the event to foo.
eval("function foo(){};  function foo::bar() {};");
}
catch(e)
{
write("event op :: " + e);
}

try
{
    // This should fail because y is not defined and we will try to register an event to it.
    eval("try {} catch(y) { function y::window() {}}");
}
catch(ex)
{
   write(ex);
}

try
{
    var obj = {};
    eval("try {} catch(obj) { function obj::window() {}}");
}
catch(ex)
{
   write(ex);
}


function TestEventHandlerSyntax() {
  arguments;
  function foo::bar() {}
}



function TestEventHandlerSyntaxWithArguments(x) {
  function x::bar() {}
}

function TestEventHandlerSyntaxWithArgumentsInObject(x) {
  arguments;
  function x::bar() {}
}

function TestEventHandlerSyntaxWithDefinedVarCore()
{
  var x = {};
  function x::y(){ }
}

function TestEventHandlerSyntaxWithDefinedVarCached()
{
  do {
    TestEventHandlerSyntaxWithDefinedVarCore();
  } while (false);
}

var globalObject = {};
function TestEventHandlerSyntaxWithDefinedArgCore()
{
  function globalObject::y(){ WScript.Echo("globalObject.y called"); }
}

function TestEventHandlerSyntaxWithDefinedArg()
{
  do {
    TestEventHandlerSyntaxWithDefinedArgCore();
  } while (false);
  globalObject.y();
}

// These methods are expected to throw since x:: is supported only while running in IE
TestMethodThrowsException("TestEventHandlerSyntax");
TestMethodThrowsException("TestEventHandlerSyntaxWithArguments");
TestMethodThrowsException("TestEventHandlerSyntaxWithArgumentsInObject");
TestMethodThrowsException("TestEventHandlerSyntaxWithDefinedArg");
