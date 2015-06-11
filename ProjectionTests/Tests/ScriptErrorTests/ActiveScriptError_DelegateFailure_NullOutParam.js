WScript.InitializeProjection();

(function () {
    'use strict'

    var testClass = new DevTests.Delegates.TestClass();
    var failure = DevTests.Delegates.FailureCondition;
    var params = DevTests.Delegates.Params;

    function delegateFunc(inspectable) {
        WScript.Echo("SHOULD NOT REACH THIS POINT");
    }

    testClass.verifyTestDelegateFailure(failure.nullOutParam, params.str, delegateFunc);

    WScript.Echo("SHOULD NOT REACH THIS POINT");
})();
