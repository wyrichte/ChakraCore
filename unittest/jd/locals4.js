//
// test arrays inspection
//

var g = {
    foo: function foo() {
        // TypedArray
        var buffer = (new Uint8ClampedArray([-256, -255, -1, 0, 1, 254, 255, 256])).buffer;
        Object.defineProperties(buffer, {
            x: { value: ArrayBuffer, enumerable: true },
            n: { value: ArrayBuffer.toString().replace(/\n/g, "").replace(/.*function (\w+)\(.*/m, "$1"), enumerable: true },
        });
        var typedArrays = [];
        [Uint8ClampedArray].forEach(function (View) {
            var v = new View(buffer);
            Object.defineProperties(v, {
                x: { value: View, enumerable: true },
                n: { value: "", enumerable: true },
            });
            typedArrays.push(v);
        });

        /**bp:locals(2, LOCALS_TYPE)**/
    }
}

g.foo(100);

WScript.Echo("pass");