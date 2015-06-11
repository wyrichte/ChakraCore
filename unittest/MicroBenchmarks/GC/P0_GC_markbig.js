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

var link = new Object();
var next = link;
for (var i = 0; i < 1000000; i++)
{
	next.blah = new Object();
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
