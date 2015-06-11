//--Start timing---------------------------------------------------------------
var d = new Date();
var temp;

for (var i = 0; i < 30000; i++) {
    try {
        throw Error("abc");
    } catch(ex) {
        temp = ex;
    }
}
recordResult((new Date() - d)/10);
//--End timing-----------------------------------------------------------------



function recordResult(timeInterval) {
    if ((temp.stack===undefined) || (temp.stack.search("abc")<0)) {
        //throw new Error("stack property was never generated for thrown Error!");
    }
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