// Float type specialization on array math.

var a = [1.1,2.2,3.3];

function f()
{
	var sum = 0;
	for(var i = 0; i < 1024*1024*5; ++i)
		sum += a[2%(i>>3)];

	return sum;
}

function time()
{
	var q;
	for(var i = 0; i < 10; ++i)
	{
		q *= f();
	}
	return q;
}
var start = new Date();
var failed = !isNaN(time());
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