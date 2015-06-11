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

var s = "abcdefghijklmnopqrstuvwxyz";
var common = new Object();

var link = new Object();
var next = link;
for (var i = 0; i < 1000000; i++)
{
	// Each object has:
	// (1) a pointer to the next object
	// (2) a pointer to its own string
	// (3) a pointer to the "common" object

	next.blah = new Object();
        next.str = s.toUpperCase();
	next.common = common;

	next = next.blah;
}

// Prime
CollectGarbage();

// Start timing
var d = new Date();
for (var i = 0; i < 10; i++)
{
	CollectGarbage();
}	
recordResult((new Date() - d)/10);
