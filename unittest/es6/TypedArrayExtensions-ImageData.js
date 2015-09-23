// ES6 ImageData TypedArray extension tests -- verifies the API shape and basic functionality

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Constructing ImageData uses Uint8ClampedArray for the data",
        body: function() {
            var data = [ 0, 1, 2, 3 ];
            var img = new ImageData( {
                width: 2,
                height: 2,
                data: data,
                size: 16,
                compression: 0.5,
                type: "text/png"
            } );

            assert.isTrue(ArrayBuffer.isView(img.data), "Object we got from ImageData.data is a TypedArray");
            assert.areEqual(Uint8ClampedArray, img.data.constructor, "Object we got from ImageData.data is an Uint8ClampedArray");
            assert.areEqual(data, img.data, "ImageData should have copied the data property into the Uint8ClampedArray");
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
