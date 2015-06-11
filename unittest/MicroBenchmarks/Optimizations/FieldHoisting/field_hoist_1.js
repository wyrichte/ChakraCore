// -------------- begin benchmark --------------
function test(o) {
    var sum = 0;
    for (var i = 1; i < 1024 * 1024 * 8 ; i++) {
        sum += o.x/i;
    }
    return sum;
}
var start = new Date();
for (var i = 0; i < 5; i++) {

    test({ x: 3.1 });
}
var end = new Date();

recordResult(end-start);
// -------------- end benchmark --------------


function recordResult(timeInterval) {
    if ((typeof WScript !== "undefined") && (typeof WScript.Echo === "function")) {
        WScript.Echo("### TIME: " + timeInterval + " ms");
    } else {
        document.getElementById("console").innerHTML = timeInterval + "ms";
        if ((window.opener) && (window.opener.recordResult)) {
            window.opener.recordResult(timeInterval);
        } else if ((window.parent) && (parent.recordResult)) {
            window.parent.recordResult(timeInterval);
        }
    }
}