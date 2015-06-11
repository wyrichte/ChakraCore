function printstack(frames) {
    for (var i in frames) {
        var frame = frames[i];
        if (frame.documentUrl != undefined) {
            frame.documentUrl = frame.documentUrl.replace(/.+unittest.StackTrace./ig, "")
        }
        else {
            frame.documentUrl = "native"
        }
        WScript.Echo(frame.functionName + " [" + frame.documentUrl + ", " + frame.line + ", " + frame.column + "]");
    }
}
function ConsoleLog(text, depth) {
    WScript.Echo(text);
    if (depth == undefined) {
        depth = 10;
    }
    printstack(diagnosticsScript.getStackTrace(depth))
}
