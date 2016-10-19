if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("IEnumerable.testMethods.js"); } 

(function () {

    runner.addTest({
        id:1,
        desc: 'IEnumerable<IMethod*> by itself - reproing bug BLUE#183883/155545',
        pri: 0,
        test: test3
    });

    Loader42_FileName = "IEnumerable_test3only";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
