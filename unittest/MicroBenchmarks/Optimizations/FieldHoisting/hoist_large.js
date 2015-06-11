
function test(o) {
    var sum = 0;
    for (var i = 1; i < 1024 * 1024 * 5 ; i++) {
        sum += 2 * o.x + 7 *  o.y + o.z + 3;
        sum -= 3 * o.x - 5 *  o.y + o.z + 3;
        sum += 4 * o.x + 8 *  o.y + o.z + 3;
        sum -= 5 * o.x - 6 *  o.y + o.z + 3;
        sum += 6 * o.x + 9 *  o.y + o.z + 3;
        sum -= 4 * o.x - 13 * o.y + o.z + 3;
    }
    return sum;
}

var start = new Date();

for (var i = 0; i < 5; i++) {
    test({ x: 3, y: 2, z: 1 });
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