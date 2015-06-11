// Tests that canvas pixel array special non-enumerated
// properties properly display in the locals display
// (length).
// Work Item: 782730
function test() {
    var uint8Arr = new Uint8Array(4);
    var pixelArr = WScript.CreateCanvasPixelArray(uint8Arr);

    // Pass true so that we can see if read only attributes are set as well.
    /**bp:evaluate('pixelArr', 1, true)**/
}

test();
WScript.Echo("PASSED");