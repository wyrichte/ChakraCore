if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); }
(function () {

    /* BEGIN IMPORTED CODE */
    /*
    Copyright (c) 2011, Daniel Guerrero
    All rights reserved.


    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.


    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DANIEL GUERRERO BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
     */


    /**
     * Uses the new array typed in javascript to binary base64 encode/decode
     * at the moment just decodes a binary base64 encoded
     * into either an ArrayBuffer (decodeArrayBuffer)
     * or into an Uint8Array (decode)
     *
     * References:
     * https://developer.mozilla.org/en/JavaScript_typed_arrays/ArrayBuffer
     * https://developer.mozilla.org/en/JavaScript_typed_arrays/Uint8Array
     */


    var Base64Binary = {
        _keyStr : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",


        /* will return a  Uint8Array type */
        decodeArrayBuffer: function(input) {
            var bytes = (input.length/4) * 3;
            var ab = new ArrayBuffer(bytes);
            this.decode(input, ab);


            return ab;
        },


        decode: function(input, arrayBuffer) {
            //get last chars to see if are valid
            var lkey1 = this._keyStr.indexOf(input.charAt(input.length-1));
            var lkey2 = this._keyStr.indexOf(input.charAt(input.length-2));


            var bytes = (input.length/4) * 3;
            if (lkey1 == 64) bytes--; //padding chars, so skip
            if (lkey2 == 64) bytes--; //padding chars, so skip


            var uarray;
            var chr1, chr2, chr3;
            var enc1, enc2, enc3, enc4;
            var i = 0;
            var j = 0;


            if (arrayBuffer) {
                uarray = new Uint8Array(arrayBuffer);
                // Modification - The script expects that the ArrayBuffer provided is zeroed out.
                for (var i = 0; i < uarray.length; ++i) {
                    uarray[i] = 0;
                }
            }
            else
                uarray = new Uint8Array(bytes);


            input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");


            for (i=0; i<bytes; i+=3) {
                //get the 3 octects in 4 ascii chars
                enc1 = this._keyStr.indexOf(input.charAt(j++));
                enc2 = this._keyStr.indexOf(input.charAt(j++));
                enc3 = this._keyStr.indexOf(input.charAt(j++));
                enc4 = this._keyStr.indexOf(input.charAt(j++));


                chr1 = (enc1 << 2) | (enc2 >> 4);
                chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
                chr3 = ((enc3 & 3) << 6) | enc4;


                uarray[i] = chr1;
                if (enc3 != 64) uarray[i+1] = chr2;
                if (enc4 != 64) uarray[i+2] = chr3;
            }


            return uarray;
        }
    }
    /* END IMPORTED CODE */

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

    runner.addTest({
        id: 5,
        desc: 'graceful failed if input buffer is not IBuffer',
        pri: '0',
        test: function () {
            var data=new Windows.Foundation.Collections.PropertySet();
            verify.exception(function () {
                new Int8Array(data);
            }, TypeError, "new Int8Array(ibuf);");
        }
    });

    Loader42_FileName = "IBuffer";

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }