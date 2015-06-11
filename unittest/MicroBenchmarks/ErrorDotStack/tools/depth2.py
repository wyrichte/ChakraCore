pre = '''function f0() {
    throw Error("abc");
}

//--Start timing---------------------------------------------------------------
var d = new Date();
var temp;

'''

mid = '''try{f0();} catch(ex) { temp = ex;}
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


with open("P1_ErrorDotStack_depth2.js", "w") as f:
    f.write(pre)
    for i in xrange(10000):
        f.write(mid)
    f.write(post)