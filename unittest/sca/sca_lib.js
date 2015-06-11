var echo = WScript.Echo;

function guarded_call(func) {
    try {
        func();
    } catch (e) {
        echo(e);
    }
}

function arrEquals(arr1, arr2) {
    if (arr1.length != arr2.length) {
        return false;
    }
    for (var i = 0; i < arr1.length; i++) {
        if (arr1[i] != arr2[i]) {
            return false;
        }
    }
    return true;
}

//
// Simple helper to get a test object from a "test" by the following sequence:
//  1. Try get its root property -- test.root
//  2. Try call its get method -- test.get()
//  3. If neither works, return test itself -- test
//
function getTestObject(test)
{
    try{
        var obj = test.root;
        if (!obj) {
            obj = test.get();
        }
        return obj;
    } catch(e) {
        return test;
    }
}

//
// Add a sequence of properties to an object. Generate property names by adding prefix
// to the property names in propValues.
//
function addProperties(obj, propValues, prefix) {
    if (prefix) {
        prefix = String(prefix);
    } else {
        prefix = "p";
    }
    var i = 1;
    for (var p in propValues) {
        obj[prefix + p] = propValues[p];
    }
}

//
// Clone an array with index and non-index properties.
//
function cloneArray(arr) {
    var o = new Array(arr.length);
    for (var p in arr) {
        o[p] = arr[p];
    }
    return o;
}

//
// fmt.print(arr): print binary data DWORDs
// fmt.stringify(root): get a string dump of an object similar to JSON but allowing cycles and more types
//
var fmt = (function(){
    var hexDigit = ["0", "1", "2", "3", "4", "5", "6", "7","8", "9", "A", "B", "C", "D", "E", "F"];

    var typedArrayDump;

    // Dump an ArrayBuffer/TypedArray/DataView as part of scadump.
    // Save the dump result in typedArrayDump if successful.
    function tryDumpTypedArray(obj, path, objArr, pathArr) {
        if (typeof ArrayBuffer === "undefined") {
            return false; // Skip if TypedArray unsupported
        }

        var s = "";
        if (obj instanceof ArrayBuffer) {
            s += "buf[";
            var arr = new Int8Array(obj);
			
            var length = arr.length ;
			if(length >= 65536)
			{
				length = arr.length% 1000; // always restrict the length to 1000 for printing, this prevents dumping a huge output for virtual arrays
			}
            for (var i = 0; i < length ; i++) {
                if (i > 0) {
                    s += ",";
                }
                s += String(arr[i]);
            }
            s += "]";
        } else {
            var typedArrays = [
                [Int8Array,     "i8"      ],
                [Uint8Array,    "u8"     ],
                [Int16Array,    "i16"     ],
                [Uint16Array,   "u16"    ],
                [Int32Array,    "i32"     ],
                [Uint32Array,   "u32"    ],
                [Float32Array,  "f32"   ],
                [Float64Array,  "f64"   ],
                [DataView,      "dv"  ]
            ];

            if (!typedArrays.some(function(arr){
                if (obj instanceof arr[0]) {
                    s += arr[1] + "[";
                    return true;
                }
            })) {
                return false;
            }

            s += scadump(obj.buffer, path + ".buffer", objArr, pathArr);
            s += ",byteOffset:" + obj.byteOffset;
            if (obj instanceof DataView) {
                s += ",byteLength:" + obj.byteLength;
            } else {
                s += ",length:" + obj.length;
            }
            s += "]";
        }

        typedArrayDump = s;
        return true;
    }

    // Dump an obj into a string for sca test.
    //  obj: The current obj node to dump. May be the root node or a descendant node.
    //  path: The property name path of obj node. e.g., "rt.a.b.c".
    //  objArr: Previously dumped nodes array, used to resolve cycles.
    //  pathArr: Corresponding property name paths for nodes in objArr.
    function scadump(obj, path, objArr, pathArr) {
        var s = "";

        var index = objArr.indexOf(obj);
        if (index >= 0) {
            s = "\"(" + pathArr[index] + ")\"";
        } else {
            objArr.push(obj);
            pathArr.push(path);

            if (typeof obj === "number" && !isFinite(obj)) {
                s += String(obj);
            }
            else if (typeof obj === "string" || obj instanceof String || obj instanceof Date) {
                s += JSON.stringify(obj);
            } else if (obj instanceof RegExp) {
                s += obj.toString();
            } else if (obj instanceof Function) {
                s += obj.toString().replace(/\n/gm, " ").replace(/\s+/gm," ").trim();
            } else if (tryDumpTypedArray(obj, path, objArr, pathArr)) {
                s += typedArrayDump;
            } else if (obj instanceof Array) {
                var count = 0;
                var lastIndex = -1;
                for (var p in obj) {
                    if (obj.hasOwnProperty(p)) {
                        var item = scadump(obj[p], path + "." + p, objArr, pathArr);

                        var i = Number(p);
                        if (i !== i || i >= obj.length || p === "") {
                            // Dump all non-index named properties
                            item = JSON.stringify(p) + ":" + item;
                            if (s.length == 0) {
                                s = "[";
                            } else {
                                s += ",";
                            }
                            s += item;
                        } else {
                            // Only dump a small number of array items in order to allow huge arrays
                            if (count < 20) {
                                count++;

                                if (s.length == 0) {
                                    s = "[";
                                } else {
                                    if (i === lastIndex + 1) {
                                        s += ",";
                                    } else if (i < lastIndex + 10) {
                                        for (var k = 0; k < i - lastIndex; k++) {
                                            s += ",";
                                        }
                                    } else {
                                        s += "..." + i + ":";
                                    }
                                }
                                lastIndex = i;
                                s += item;
                            } else if (count === 20) {
                                count++;
                                s += ("...(" + obj.length + ")");
                            }
                        }
                    }
                }
                if (s.length > 0) {
                    s += "]";
                } else if (obj.length > 0) {
                    s = "[...(" + obj.length + ")]";
                } else {
                    s = JSON.stringify(obj);
                }
            } else if (obj instanceof Map) {
                s = "{ (Map) { ";
                obj.forEach(function (v, k) {
                    if (s.length > 10) {
                        s += ","
                    }
                    // path here is incorrect, but it works well enough for testing
                    s += "[" + scadump(k, path, objArr, pathArr) + ":" + scadump(v, path, objArr, pathArr) + "]";
                });
                if (s.length > 10) {
                    s += " }";
                } else {
                    s += "}";
                }
                for (var p in obj) {
                    if (obj.hasOwnProperty(p)) {
                        s += "," + (JSON.stringify(p) + ":" + scadump(obj[p], path + "." + p, objArr, pathArr));
                    }
                }
                s += " }";
            } else if (obj instanceof Set) {
                s = "{ (Set) { ";
                obj.forEach(function (v) {
                    if (s.length > 10) {
                        s += ","
                    }
                    // path here is incorrect, but it works well enough for testing
                    s += scadump(v, path, objArr, pathArr);
                });
                if (s.length > 10) {
                    s += " }";
                } else {
                    s += "}";
                }
                for (var p in obj) {
                    if (obj.hasOwnProperty(p)) {
                        s += "," + (JSON.stringify(p) + ":" + scadump(obj[p], path + "." + p, objArr, pathArr));
                    }
                }
                s += " }";
            } else {
                for (var p in obj) {
                    if (obj.hasOwnProperty(p)) {
                        if (s.length == 0) {
                            s = "{";
                        } else {
                            s += ",";
                        }
                        s += (JSON.stringify(p) + ":" + scadump(obj[p], path + "." + p, objArr, pathArr));
                    }
                }
                if (s.length > 0) {
                    s += "}";
                } else {
                    s += JSON.stringify(obj);
                }
            }
        }

        return s;
    }

    return {
        // Convert a byte value to hex string
        toHex: function(byte) {
            var low = byte % 16;
            var high = Math.floor(byte / 16);
            return hexDigit[high] + hexDigit[low];
        },

        // Convert 4 bytes to a hex string (little endian byte order)
        toHexDW: function(b0, b1, b2, b3) {
            return this.toHex(b3) + this.toHex(b2) + this.toHex(b1) + this.toHex(b0);
        },

        // Print bytes array as DWORDs (little endian byte order).
        print: function(arr) {
            var len = arr.length;
            var s = "";

            var unalignedLen =  len % 4;
            len -= unalignedLen;

            for (var i = 0; i < len; i += 4) {
                if (i % 16 == 0) {
                    if (i > 0) {
                        echo(s);
                        s = "";
                    }
                }
                if (s.length > 0)
                {
                    s += " ";
                }
                s += this.toHexDW(arr[i], arr[i + 1], arr[i + 2], arr[i + 3]);
            }

            if (unalignedLen > 0) {
                for (var i = 0; i < unalignedLen; i++) {
                    if (s.length > 0)
                    {
                        s += " ";
                    }
                    s += this.toHex(arr[len + i]);
                }
            }

            if (s.length > 0) {
                echo(s);
            }
        },

        // Get a string dump of an object similar to JSON but allowing cycles
        stringify: function(root) {
            return scadump(root, "rt", [], []);
        }
    };
})();
