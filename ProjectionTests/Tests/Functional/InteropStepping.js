if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var setTimeOutCalls = 0;
    var savedSetTimeout = undefined;
    if (typeof setTimeout !== 'undefined')
        savedSetTimeout = setTimeout;
    runner.globalSetup(function () {
        setTimeout = function () { setTimeOutCalls = setTimeOutCalls + 1; }
    });
    runner.globalTeardown(function () {
        setTimeout = savedSetTimeout;
    });

    /**bp:trackProjectionCall()**/

    runner.addTest({
        id: '1',
        desc: 'Call all instrumented WinRT callout points.',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(1);

            // Call methods exercising all of the fast-path WinRT callouts
            /**bp:trackProjectionCall("fastPath")**/
            a.fastPath();
            /**bp:trackProjectionCall("fastPathIn")**/
            a.fastPathIn(16);
            /**bp:trackProjectionCall("fastPathOut")**/
            a.fastPathOut();
            /**bp:trackProjectionCall("fastPathInOut")**/
            a.fastPathInOut(16);
            /**bp:trackProjectionCall("fastPathInIn")**/
            a.fastPathInIn(16, 16);

            // Call methods exercising all of the static fast-path WinRT callouts
            /**bp:trackProjectionCall("staticFastPath")**/
            Animals.Animal.staticFastPath();
            /**bp:trackProjectionCall("staticFastPathIn")**/
            Animals.Animal.staticFastPathIn(16);
            /**bp:trackProjectionCall("staticFastPathOut")**/
            Animals.Animal.staticFastPathOut();
            /**bp:trackProjectionCall("staticFastPathInOut")**/
            Animals.Animal.staticFastPathInOut(16);
            /**bp:trackProjectionCall("staticFastPathInIn")**/
            Animals.Animal.staticFastPathInIn(16, 16);

            // Call methods exercising slow-path WinRT callouts
            /**bp:trackProjectionCall("slowPath")**/
            a.slowPath(1, 2, 3, 4, 5, 6);
            /**bp:trackProjectionCall("staticSlowPath")**/
            Animals.Animal.staticSlowPath(1, 2, 3, 4, 5, 6);

            // Call EventHandler Add/Remove
            var handler = function (sender, val) { }
            /**bp:trackProjectionCall("addEventListener")**/
            a.addEventListener('eventhandler', handler);
            /**bp:trackProjectionCall("removeEventListener")**/
            a.removeEventListener('eventhandler', handler);

            // Call ArrayBuffer from IBuffer path
            var length = 42;
            /**bp:trackProjectionCall("new Windows.Storage.Streams.Buffer")**/
            var buffer = new Windows.Storage.Streams.Buffer(length);
            /**bp:trackProjectionCall("new Uint8Array")**/
            var readerwriter = new Uint8Array(buffer, 0, length);
        }
    });

    Loader42_FileName = "Interop Stepping tests";
})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
