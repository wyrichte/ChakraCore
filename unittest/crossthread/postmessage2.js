
// Exhibiting WScript.postMessage behaviour with Array buffer
// Use WScript.Done to stop process any more messages.


var sc1 = WScript.LoadScript(`
onmessage = function(e) {
    var ab = e.data;
    var ta = new Int32Array(ab);
    if (ta[0] != 20) {
        print('failed : ' + ta[0]);
    }
    ta[1] = 30;
    WScript.postMessage(undefined, ab, [ab]);
}
`, "crossthread");

var ab = new ArrayBuffer(16);
var ta = new Int32Array(ab);
ta[0] = 20;
WScript.postMessage(sc1, ab, [ab]);

try {
    ta[0];
    print('Failed - above code throws exception stating Array is detached');
} catch(ex) {
}

onmessage = function(e) {
    var ta = new Int32Array(e.data);
    if (ta[0] != 20 || ta[1] != 30) {
        print('failed : ' + ta[1]);
    } else {
        print('pass');
    }
    WScript.Done();
}
