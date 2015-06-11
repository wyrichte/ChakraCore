pre = '''function f0() {
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

function f12_4(n) {
    if (n===4) {
        return f3();
    }
    return f12_4(n-1);
}

var f13 = new Function("'use strict'; return f12_4(12);");

Error.stackTraceLimit = -1;

//--Start timing---------------------------------------------------------------
var d = new Date();
var temp;

'''

mid = '''try{ eval("f13();");} catch(ex){ temp = ex;}
'''

post = '''
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
'''


with open("P1_ErrorDotStack_depth16.js", "w") as f:
    f.write(pre)
    for i in xrange(20000):
        f.write(mid)
    f.write(post)