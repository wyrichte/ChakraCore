try {
    var e = Error("123");
    e.somevalue = "xyz";
    e.stack = "abacaba";
    for (var p in e) {
        WScript.Echo(p + " = " + e[p]);
    }
    
    throw e;
}
catch (ex) {
    WScript.Echo("----------------------");
    for (var p in ex) {
        WScript.Echo(p + " = " + ex[p]);
    }
}