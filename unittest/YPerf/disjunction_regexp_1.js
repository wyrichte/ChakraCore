// BEGIN PRELUDE
function echo(o) {
    try {
        document.write(o + "<br/>");
    } catch (ex) {
        try {
            WScript.Echo("" + o);
        } catch (ex2) {
            print("" + o);
        }
    }
}

var _startDate = new Date();
// END PRELUDE

function RunTest() {
    var count = 200000;
    var testString = "ab";
    var testStrings = [testString, testString + " "];
    var re = /(ab) | bc | def/;
    for (var i = 0; i < count; i++) {
        var matchString = testStrings[i % 2];
        var result = re.exec(matchString);
    }
}

// BEGIN POSTLUDE
RunTest();
var _interval = new Date() - _startDate;

echo("### TIME: " + _interval + " ms");
// END POSTLUDE