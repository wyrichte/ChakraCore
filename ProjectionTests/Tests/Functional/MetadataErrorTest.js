if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    var description = ": an unexpected failure occurred while trying to obtain metadata information";
    
    var metadataErrors = {
        InvalidMetadataFile: "(0x80000012)",
        BadMetadata: "(0x8013118A)",
        BadSignature: "(0x80131192)",
        InternalError: "(0x80131FFF)",
        OutOfMemory: "(0x8007000E)",
        Fail: "(0x80004005)"
    }

    runner.addTest({
        id: 1,
        desc: 'Metadata Errors',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            for (var error in metadataErrors) {
                logger.comment("Test MetadataError." + error);
                try {
                    var x = MetadataError[error];
                    fail("Metadata error expected: " + error)
                }
                catch (e) {
                    verify.instanceOf(e, Error);
                    var message = "MetadataError." + error + " " + metadataErrors[error] + description;
                    verify(e.message, message, "e.message");
                }
            }
        }
    });

    Loader42_FileName = "Metadata Errors Test";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
