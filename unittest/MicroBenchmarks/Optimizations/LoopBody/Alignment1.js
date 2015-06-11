// -------------- begin benchmark --------------
// Verifies that loop bodies have a double aligned stack. (WIN8:494585)
function f(x)
{
      var sum = 0;
      for(var i = 0; i < 20*1024*1024.1; i += 1.01)
            sum += i/x;
      return sum;
}
var failed = false;
var start = new Date();
for (var i = 0; i < 5; i++) {

    if(f(3.7) != 58856106127495.38)
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