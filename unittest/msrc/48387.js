var str = "Test Regexp control char\\";
var backSlash = String.fromCharCode(92);
var passed = false;
try {
    var re = new RegExp('[\\c' + backSlash + ']');
    var result = re.exec(str);
}
catch (e) {
    // expected error: SyntaxError: Expected ']' in regular expression
    passed = true;
}

if (passed) {
    WScript.Echo("passed");
}
