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
    var re = /(((ab)|bc).|(de)){2,4}/;
    var testString = "abxdebcxde";
    var testStrings = [testString, testString + " "];
    for (var i = 0; i < count; i++) {
        var matchString = testStrings[i % 2];
        var result = re.test(matchString);
    }
}

// BEGIN POSTLUDE
RunTest();
var _interval = new Date() - _startDate;

echo("### TIME: " + _interval + " ms");
// END POSTLUDE