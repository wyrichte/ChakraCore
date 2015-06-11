if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("IEnumerable.testMethods.js"); } 

(function () {

    runner.addTest({
        id:1,
        desc: 'IEnumerable<Interface> - reproing bug BLUE#232596',
        pri: 0,
        test: test2
    });

    Loader42_FileName = "IEnumerable_test2only";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
