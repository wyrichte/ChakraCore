WScript.RegisterCrossThreadInterfacePS();

function runTest(o) {
    try {
        WScript.Echo(Object.isExtensible(o));
    }
    catch (e) {
        WScript.Echo(e.message);
    }

    try {
        WScript.Echo(Object.preventExtensions(o));
    }
    catch (e) {
        WScript.Echo(e.message);
    }

    try {
        WScript.Echo(Object.isExtensible(o));
    }
    catch (e) {
        WScript.Echo(e.message);
    }
    WScript.Echo();
}

function OneIteration(x)
{
runTest(x);
runTest(x.testUndefined);
runTest(x.testNull);
runTest(x.testObject);
runTest(x.testBoolean);
runTest(x.testBooleanObject);
runTest(x.testDate);
runTest(x.testNumber1);
runTest(x.testNumber2);
runTest(x.testNumber3);
runTest(x.testNumberObject1);
runTest(x.testNumberObject2);
runTest(x.testNumberObject3);
runTest(x.testRegExp);
runTest(x.testArray);
runTest(x.testFunction);
runTest(x.testString);
runTest(x.testStringObject);
}

WScript.Echo("crossthread");
var childWin = WScript.LoadScriptFile("testObjects.js", "crossthread");
OneIteration(childWin);

var childFrame = WScript.LoadScriptFile("testObjects.js", "samethread");
WScript.Echo("sameThread");
OneIteration(childFrame);
WScript.Shutdown(childFrame);
WScript.Echo("sameThread after close");
OneIteration(childFrame);
