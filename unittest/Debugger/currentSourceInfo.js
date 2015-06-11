function trimPath(line) {
    return line && line.replace(/.+unittest.Debugger./ig, "");
}

function dump(info) {
    WScript.Echo(trimPath(info.url), info.line + 1, info.column + 1);
}

dump(Debug.getCurrentSourceInfo());
dump(Debug.getCurrentSourceInfo());

for (var i = 0; i < 2; i++) {
    dump(Debug.getCurrentSourceInfo());
}

(function foo() {
    var a = [
        0,
        Debug.getCurrentSourceInfo(),
        2];
    dump(a[1]);
})();

dump(eval("Debug.getCurrentSourceInfo()"));

var f = new Function("return Debug.getCurrentSourceInfo()");
dump(f());
