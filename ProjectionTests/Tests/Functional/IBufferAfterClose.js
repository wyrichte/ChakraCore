if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); }
(function () {

    runner.addTest({
        id: 0,
        desc: 'BLUE 178101: IBuffer use after ScriptSite closed',
        pri: '0',
        test: function () {
            var child = WScript.LoadScriptFile("IBufferAfterCloseChild.js", "samethread");
            var ccibuf = child.ibuf;

            WScript.Shutdown(child);

            verify.exception(function () {
                ccibuf.length;
            }, TypeError, "IBuffer length property fails after close");
            
            verify.exception(function () {
                ccibuf.capacity;
            }, TypeError, "IBuffer capacity property fails after close");

            verify.exception(function () {
                new Int8Array(ccibuf);
            }, TypeError, "IBuffer cannot be used as a TypedArray after close");
        }
    });

    Loader42_FileName = "IBuffer After Close";

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
