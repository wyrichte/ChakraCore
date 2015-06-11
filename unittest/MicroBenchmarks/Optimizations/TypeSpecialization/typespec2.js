// Type specialization on array math with a constant. (WIN8:526969)
var ary1 = new Array(1024);

for(var i = 0; i < 1024; ++i)
{
	ary1[i] = i%4;
}

function test(a,b)
{
	var sum = 0;

	for(var j = 0; j < 1024 * 5; ++j)
	{
		for(var i = 0; i < 1024; ++i)
		{
			sum += 4 % ary1[39];

			if(sum > 1000)
				sum = 0;
		}
	}

	return sum;
}

var failed = false;
var start = new Date();
for(var i = 0; i < 5; ++i)
{
	if(test() != 643)
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