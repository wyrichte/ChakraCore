var expectedError = "İ";

function test() {
    "İ";
}

var info = Debug.getCurrentSourceInfo();
WScript.Echo(info.line + ',' + info.column);
