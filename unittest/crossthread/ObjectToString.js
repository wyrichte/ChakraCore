WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");

function runTest(o) {
    try {
        WScript.Echo(o.toString());
    }
    catch (e) {
        WScript.Echo(e.message);
    }

    try {
        WScript.Echo(o.toLocaleString());
    }
    catch (e) {
        WScript.Echo(e.message);
    }

    try {
        WScript.Echo(Object.prototype.toString.call(o));
    }
    catch (e) {
        WScript.Echo(e.message);
    }

    try {
        WScript.Echo(Object.prototype.toLocaleString.call(o));
    }
    catch (e) {
        WScript.Echo(e.message);
    }
    WScript.Echo();
}

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
