function fRecurse(n) {
    if (n===0) {
        throw Error("abc");
    }
    return fRecurse(n-1);
}

Error.stackTraceLimit = -1;

//--Start timing---------------------------------------------------------------
var d = new Date();
var temp;

for (var i=0; i<200; i++) {
    try {
        fRecurse(1021);
    } catch(ex){ 
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
