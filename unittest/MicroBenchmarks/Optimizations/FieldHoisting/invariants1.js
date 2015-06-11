// Hoist a math operation (MUL) that operates on a field that is invariant through the inner loop. (WIN8:524027)

var failed = false;

function test(o) {
	var sum = 0;

	for(var j = 0; j < 100; ++j) {
		o.x *= 2;
		for (var i = 1; i < 1024 * 200 ; i++) {
			sum += o.x * 5;
		}
	}

	if(sum != 7.788407258284235e+36)
		failed = true;

}

var start = new Date();
for (var i = 0; i < 5; i++) {
	test({ x: 3 });
}
var end = new Date();

recordResult(failed ? Infinity : end-start);
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