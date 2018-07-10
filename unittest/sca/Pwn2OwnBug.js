function serialize(rootObject, transferArgs) {
    return SCA.serialize(rootObject, { context: "samethread" } , undefined, transferArgs);
}

function deserialize(data) {
    return SCA.deserialize(data);
}


function neuter(val) {
    deserialize(serialize(null, [val]));
}

function test1() {
    var ab = new ArrayBuffer(0x1000000);
    try {
        var dv = new DataView(ab, { "valueOf": function () { neuter(ab); return 0; } }); // test case 1	
        WScript.Echo('Expected to throw.');
    } catch (e) {
        if (e instanceof TypeError) {
            if (e.message !== "The ArrayBuffer is detached.") {
                WScript.Echo('FAIL ' + e.message);
            }
        } else {
            WScript.Echo('FAIL ' + e.message);
        }
    }
}

function test2() {
    var ab = new ArrayBuffer(0x1000000);
    try {
        var dv = new DataView(ab, 0, { "valueOf": function () { neuter(ab); return 0xFFFFFF; } }); // test case 2
        WScript.Echo('Expected to throw.');
    } catch (e) {
        if (e instanceof TypeError) {
            if (e.message !== "The ArrayBuffer is detached.") {
                WScript.Echo('FAIL ' + e.message);
            }
        } else {
            WScript.Echo('FAIL ' + e.message);
        }
    }
}

test1();
test2();
WScript.Echo("PASS");