function testEval(str) {
  try {
    WScript.Echo(eval(str));
  } catch (e) {
    WScript.Echo(e);
  }
}

function testFuncConstructor(str) {
    try{
        WScript.Echo(new Function(str)());
    } catch (e) {
        WScript.Echo(e);
    }
}

function performTest() {
    WScript.SetEvalEnabled(false);
    testFuncConstructor("return 5;");

    WScript.SetEvalEnabled(true);
    testFuncConstructor("return 5;");

    WScript.SetEvalEnabled(false);
    testFuncConstructor("return 5;");

    WScript.SetEvalEnabled(false);
    testEval("5 + 5;");

    WScript.SetEvalEnabled(true);
    testEval("5 + 5;");

    WScript.SetEvalEnabled(false);
    testEval("5 + 5;");

    WScript.SetEvalEnabled(false);
    var executedOuter = false;
    var executedInner = false;
    function forJit() {
        (function () {
            executedOuter = true;
            function a() {
                executedInner = true;
            }
            a();
        }());
    }
    forJit();
    forJit();
    forJit();
    forJit();

    if (!executedOuter) {
        WScript.Echo("Didn't execut outer.");
    }

    if (!executedInner) {
        WScript.Echo("Didn't execut inner.");
    }

    WScript.SetEvalEnabled(true);
    var arr = [];
    for (var i = 0; i < 10; i++) {
        arr.push("WScript.SetEvalEnabled(false); testEval('" + i + ";'); WScript.SetEvalEnabled(true);'Test'");
    }
    arr.map(testEval);
    WScript.SetEvalEnabled(true);
    var arr = [];
    for (var i = 0; i < 10; i++) {
        arr.push("WScript.Echo(" + i + ");");
    }
    arr.map(eval);
}

performTest();//For jit
performTest();
WScript.SetRestrictedMode(false);
performTest();