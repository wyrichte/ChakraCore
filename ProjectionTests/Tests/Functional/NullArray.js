if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); }
(function () {
    runner.addTest({
        id: 0,
        desc: 'Bug 149800 - AV in ReadOutArrayType with a null array',
        pri: '0',
        test: function () {
            var a=Windows.Security.Cryptography.Certificates.CertificateQuery();
            a.thumbprint=null;
            a.thumbprint;
        }
    });
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }