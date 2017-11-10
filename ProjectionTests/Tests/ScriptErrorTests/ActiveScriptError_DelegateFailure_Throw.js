WScript.InitializeProjection();

(function () {
    'use strict'

    var testClass = new DevTests.Delegates.TestClass();
    var failure = DevTests.Delegates.FailureCondition;
    var params = DevTests.Delegates.Params;

    function delegateFunc(inspectable) {
        throw new Error();
    }

    testClass.verifyTestDelegateFailure(failure.jsException, params.none, delegateFunc);

    WScript.Echo("SHOULD NOT REACH THIS POINT");
})();
