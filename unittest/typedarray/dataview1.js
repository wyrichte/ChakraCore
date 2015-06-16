function test1() {
    var intArray = Array(0x100); //[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
    var arrayBuffer = (new Uint32Array(intArray)).buffer;
    var viewStart = 0;
    var viewLength = arrayBuffer.byteLength;
    var view = new DataView(arrayBuffer, viewStart, viewLength);
    for (var i = 0; i <= 8; i++) {
        try {
            WScript.Echo('view.getUint32(-' + i + '): 0x' + view.getUint32(-i).toString(16));
        } catch (e) {
            WScript.Echo(e.message);
        }
    }
    for (var i = 0; i <= 8; i++) {
        try {
            WScript.Echo('view.setUint32(-' + i + '): 0x' + view.setUint32(-i, 10).toString(16));
        } catch (e) {
            WScript.Echo(e.message);
        }
    }
}

function test2() {
    var arrayBuffer = new ArrayBuffer(10);

    try{
        var view1 = new DataView(arrayBuffer, undefined);
    } catch (e) {
        if (e instanceof RangeError) {
            if(e.message !== "DataView constructor argument offset is invalid"){
                WScript.Echo('FAIL');
            }
        } else {
            WScript.Echo('FAIL');
        }
    }

    try{
        var view2 = new DataView(arrayBuffer, 1.5);
    } catch (e) {
        if (e instanceof RangeError) {
            if (e.message !== "DataView constructor argument offset is invalid") {
                WScript.Echo('FAIL');
            }
        } else {
            WScript.Echo('FAIL');
        }
    }
    WScript.Echo('PASS');
}

function test3() {
    var v1 = new DataView(new ArrayBuffer(), 0, 0);
    var v2 = new DataView(new ArrayBuffer(1), 1, 0);
}

test1();
test2();
test3();
