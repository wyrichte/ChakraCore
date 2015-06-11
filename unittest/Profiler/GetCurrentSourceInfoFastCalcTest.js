var firstLine = Debug.getCurrentSourceInfo();

function dumpLineInfo(info) {
    WScript.Echo(info.line);
    WScript.Echo(info.column);
}

dumpLineInfo(firstLine);

function test() {
    var functionLine = Debug.getCurrentSourceInfo();
    dumpLineInfo(functionLine);

    for (var i = 0; i < 3; ++i) {
        var loopLine = Debug.getCurrentSourceInfo();
        dumpLineInfo(loopLine);
    }
}

test();
WScript.Echo(Debug.getCurrentSourceInfo().line + "," + Debug.getCurrentSourceInfo().column);