// Float type specialization with an assignment to an integer in the loop. (WIN8:547648)

function f()
{
	var sum = 0;

	for(var i = 0; i < 1024 * 1024 * 20; ++i)
	{
		sum += 0.1;
		
		if(sum > 5)
			sum = 0;
	}

	return sum;
}

var failed = false;
var start = new Date();
for(var i = 0; i < 5; ++i)
{
	if(f() != 1.4000000000000001)
		failed = true;
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