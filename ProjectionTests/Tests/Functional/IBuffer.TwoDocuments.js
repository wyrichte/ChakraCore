if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); }

// import second document -- see BLUE #190492
if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { 
  WScript.LoadScriptFile("IBuffer.TwoDocuments.library.js"); 
}

(function () {
    function getBase64DecodedStringLength(base64String) {
        var len = base64String.length;

        len = len * 3 / 4;
        if (base64String.charAt(base64String.length-1) == '=') {
            len--;
        }
        if (base64String.charAt(base64String.length-2) == '=') {
            len--;
        }

        len++; // for \0

        return len;
    }

    runner.addTest({
        id: 0,
        desc: 'Base64Decode using IBuffer',
        pri: '0',
        test: function () {
            var base64String = 'dGVzdGluZyB0ZXN0aW5nIDEyMw==';
            var bufferLength = getBase64DecodedStringLength(base64String);
            verify.equal(bufferLength, 0x14, "bufferLength");
            var buffer = new Windows.Storage.Streams.Buffer(bufferLength);
            var uint8Array = Base64Binary.decode(base64String, buffer);
            var dataReader = Windows.Storage.Streams.DataReader.fromBuffer(buffer);
            var decodedString = dataReader.readString(bufferLength);
            if (decodedString == 'testing testing 123') pass();
        }
    });

    runner.addTest({
        id: 1,
        desc: 'IBuffer direct read/write access',
        pri: '0',
        test: function () {
            var buffer = new Windows.Storage.Streams.Buffer(42);
            var readerwriter = new Uint8Array(buffer, 0, 42);
            var sum = 0;
            for (var i = 0; i < 42; ++i) {
                buffer[i] = i;
                sum += i;
            }
            for (var i in buffer) {
                sum -= i;
            }
            if (sum == 0) pass();
        }
    });

    runner.addTest({
        id: 2,
        desc: 'IBuffer properties and projection aliasing',
        pri: '0',
        test: function () {
            var length = 42;
            var buffer = new Windows.Storage.Streams.Buffer(length);
            // When we initialize a buffer, the length should be 0
            verify(buffer.capacity, length, 'Initial buffer capacity is correct');
            verify(buffer.length, 0, 'Initial buffer length is correct');
            // Buffer should project byteLength
            verify(buffer.byteLength, length, 'Buffer projects injected byteLength property');

            var readerwriter = new Uint8Array(buffer, 0, length);
            // Now that the ArrayBuffer IBuffer projection has happened, buffer length should be 42
            verify(buffer.length, length, 'Buffer length is equal to capacity after projection');
            verify(readerwriter.byteLength, length, 'Projected ArrayBuffer length is correct');

            // Modifications to the buffer should not change the length
            readerwriter[7] = 'a';
            verify(buffer.length, length, 'Buffer length is the same after write');
            verify(buffer.byteLength, length, 'byteLength is unchanged after write');
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Blue Bug 175495: IBuffer Dataview Support',
        pri: '0',
        test: function () {
            var view = new DataView(new Windows.Storage.Streams.Buffer(10));
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Blue Bug 176611: Use of ArrayBufferFromIBuffer after IBuffer release',
        pri: '0',
        test: function () {
            var ibuf=new Windows.Storage.Streams.Buffer(10);
            msReleaseWinRTObject(ibuf);
            verify.exception(function () {
                new Int8Array(ibuf);
            }, TypeError, "new Int8Array(ibuf);");
        }
    });

    Loader42_FileName = "IBuffer (Two documents)";

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }