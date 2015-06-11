var ary1 = new Array(1024);
for(var i = 0; i < 1024; ++i)
{
	ary1[i] = i%4;
}

function test(a,b)
{
	var sum = 0;

	for(var j = 0; j < 1024 * 50; ++j)
	{
		for(var i = 0; i < 1024; ++i)
		{
			sum += ary1[i] % ary1[39];


		}
	}

	return sum;
}

var start = new Date();

for(var i = 0; i < 5; ++i)
{
	test(i,i+0.1);
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