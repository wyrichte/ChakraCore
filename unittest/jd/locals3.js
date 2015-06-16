//
// test arrays inspection
//

function Animal() { }

var g = {
    a: "this member",
    foo: function foo(y, x) {
        // Empty array
        var arr0 = [];

        // Simple array
        var arr1 = [0, 1, , 3];

        // Array with loop and named properties
        var arr2 = [0, 1, 2];
        arr2[3] = arr1;
        arr2[4] = arr2; // loop!
        arr2.length = 6;
        arr2.x = Animal;
        arr2.a = "a";

        // Very sparse array
        var arr3 = [];
        arr3[100] = 100;
        arr3[10000] = 10000;
        arr3[0xFFFFFFFE] = 0xFFFFFFFE;
        arr3[0xFFFFFFFF] = 0xFFFFFFFF;

        // Object with inner array
        var obj0 = {
            "-1": -1,
            "0": 0,
            "10": 10,
            "2": 2,
            "3": undefined,
            "4": Animal,
            x: Animal,
            a: "a",
        };

        // Items from prototype chain
        var arr4 = [0];
        arr4.length = 5;
        arr4.__proto__ = {
            "0": "v0",
            "1": "v1",
            __proto__: [10, 11, 12, 13, 14]
        };

        // ES5Array
        var arr5 = [0, 1, 2, , 4];
        Object.defineProperties(arr5, {
            2: { value: "2:nonEnumerable", enumerable: false },
            3: { get: function () { return "getter"; }, enumerable: true, configurable: true },
            x: { value: Animal, enumerable: true },
            a: { value: "a", enumerable: true },
        });

        // Items from prototype chain, and object contains internal ES5 array
        var arr6 = [0];
        arr6.length = 5;
        arr6.__proto__ = {
            0: "v0",
            1: "v1",
            __proto__: [10, 11, 12, 13, 14]
        };
        Object.defineProperties(arr6.__proto__, {
            2: { value: "2:nonEnumerable", enumerable: false },
            3: { get: function () { return "getter"; }, enumerable: true, configurable: true }, // shouldn't display 13 in Value
            x: { value: Animal, enumerable: true },
            a: { value: "a", enumerable: true },
        });

        // TypedArray
        var buffer = (new Int8Array([0, 1, 2, 3, 0, -1, -2, -3])).buffer;
        Object.defineProperties(buffer, {
            x: { value: ArrayBuffer, enumerable: true },
            n: { value: ArrayBuffer.toString().replace(/\n/g, "").replace(/.*function (\w+)\(.*/m, "$1"), enumerable: true },
        });
        var typedArrays = [];
        [DataView, Int8Array, Uint8Array, Int16Array, Uint16Array, Int32Array, Uint32Array, Float32Array, Float64Array].forEach(function (View) {
            var v = new View(buffer);
            Object.defineProperties(v, {
                x: { value: View, enumerable: true },
                n: { value: "", enumerable: true },
            });
            typedArrays.push(v);
        });

        // JavascriptPixelArray
        var pixelArr = (function () {
            var img = new ImageData({
                width: 2,
                height: 2,
                data: [2, 3, 4, 5], // PixelArray verifies length % 4 == 0
            });
            var data = img.data;
            Object.defineProperties(data, {
                x: { value: data.constructor, enumerable: true },
                n: { value: "n", enumerable: true },
            });
            return data;
        })();

        // JavascriptDate, VariantDate
        var dates = (function () {
            var o = {
                date0: new Date("2013-03-14T22:14:36.798Z"),
                date1: new Date(NaN)
            };
            o.vardate0 = o.date0.getVarDate();
            o.vardate1 = o.date1.getVarDate();
            return o;
        })();

        var regex0 = /abcd/;
        var regexCon = regex0.constructor;

        /**bp:locals(2, LOCALS_TYPE)**/
    }
};
g.foo(123, "456", 789);

WScript.Echo("pass");