function test(o,p,q) {
    var sum = 0;
    for (var i = 1; i < 1024 * 1024 * 5; i++) {
        if (i & 3 == 0) {
            o.x += p.y;
        }
        else if (i & 3 == 1) {
            o.x -= q.y;
        }
        else {
            p.y--;
            q.y--;

            if (p.y < -100) {
                p.y = 0;
                q.y = 0;
            }
        }
    }
    return sum;
}

var start = new Date();

for (var i = 0; i < 5; i++) {
    var o = { x: 3, y: 2, z: 1 };
    var p = { x: 3, y: 2, z: 1 };
    var q = { x: 3, y: 2, z: 1 };


    test(o,p,q);
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