WScript.InitializeProjection();

(function () {
    'use strict'

    var testClass = new DevTests.Delegates.TestClass();
    var failure = DevTests.Delegates.FailureCondition;
    var params = DevTests.Delegates.Params;

    function delegateFunc(inspectable) {
        return { str: "hello", num: 42, rc: testClass, iface: new Animals.Animal() };
    }

    testClass.verifyTestDelegateFailure(failure.failToMarshal, params.iface, delegateFunc);

    WScript.Echo("SHOULD NOT REACH THIS POINT");
})();
