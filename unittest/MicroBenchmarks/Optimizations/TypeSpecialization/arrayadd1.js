// Type specialization on array math. (WIN8:513168)

var N = 30000;
var Count = 1000;
var ga1 = new Array(N);
var ga2 = new Array(N);
var ga3 = new Array(N);
for(var i = 0; i < N; ++i)
{
	ga1[i] = i%5;
	ga2[i] = i%4;
	ga3[i] = 0;
}

function f(a1,a2,a3)
{
	for(var x = 0; x < Count; ++x)
	{
		for(var i = 0; i < N; ++i)
		{
			a3[i] = a2[i] + a1[i];
		}
	}
}

var start = new Date();
for(var i = 0; i < 6; ++i)
{
	f(ga1,ga2,ga3);
}
var end = new Date();

var sum = 0;
for(var i = 0; i < N; ++i)
	sum += ga3[i];

recordResult(sum != 105000 ? Infinity : end-start);
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