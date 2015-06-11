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
            if (e.message !== "'this' is not a DataView object") {
                WScript.Echo('FAIL');
            }
        } else {
            WScript.Echo('FAIL');
        }
    }
}

function test2() {
    var ab = new ArrayBuffer(0x1000000)
    var dv = new DataView(ab, 0, { "valueOf": function () { neuter(ab); return 0xFFFFFF; } }); // test case 2
    if (dv.byteOffset != 0 && dv.byteLength != 0) {
        WScript.Echo("FAIL");
    }
    try
    {
        dv.getInt32(0x31337, true);
        WScript.Echo('Expected to throw.');
    } catch (e) {
        if (e.message !== 'DataView.prototype.GetInt32: The ArrayBuffer is detached.') {
            WScript.Echo("FAIL" + e.message);
        }
    }
    
}

test1();
test2();
WScript.Echo("PASS");