// Tests that typed array special non-enumerated
// properties properly display in the locals display
// (length, buffer, byteOffset, byteLength, BYTES_PER_ELEMENT).
// Work Item: 782730
function test() {
    var int8Arr = new Int8Array(1);
    var uint8Arr = new Uint8Array(2);
    var int16Arr = new Int16Array(3);
    var uint16Arr = new Uint16Array(4);
    var int32Arr = new Int32Array(5);
    var uint32Arr = new Uint32Array(6);
    var float32Arr = new Float32Array(7);
    var float64Arr = new Float64Array(8);

    // Pass true so that we can see if read only attributes are set as well.
    /**bp:evaluate('int8Arr', 2, true)**/
    /**bp:evaluate('uint8Arr', 2, true)**/
    /**bp:evaluate('int16Arr', 2, true)**/
    /**bp:evaluate('uint16Arr', 2, true)**/
    /**bp:evaluate('int32Arr', 2, true)**/
    /**bp:evaluate('uint32Arr', 2, true)**/
    /**bp:evaluate('float32Arr', 2, true)**/
    /**bp:evaluate('float64Arr', 2, true)**/
}

test();
WScript.Echo("PASSED");