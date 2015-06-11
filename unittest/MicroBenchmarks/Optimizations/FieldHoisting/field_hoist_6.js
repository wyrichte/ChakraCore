//Field hoisting including check/branches. (WIN8:544789)

var obj = {x: 3, y: 4};

function test()
{
	var sum = 0;

	for(var j = 0; j < 1024 * 50; ++j)
	{
		for(var i = 0; i < 1024; ++i)
		{
			if(i&1)
				sum += i * obj.y;

			if(sum > 1000 || sum < -1000)
				sum = 0;
		}
	}

	return sum;
}
var failed = false;
var start = new Date();
for(var i = 0; i < 5; ++i)
{
	if(test() != 0)
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
