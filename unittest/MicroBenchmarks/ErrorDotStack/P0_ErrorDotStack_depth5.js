function f0() {
    throw Error("abc");
}

var f1 = function() {
    f0();
}

var f2 = Function("return f1();");

function f3() {
    "use strict";
    f2();
}


//--Start timing---------------------------------------------------------------
var d = new Date();
var temp;

for (var i = 0; i < 20000; i++) {
    try {
        f3();
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