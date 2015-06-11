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

function RunTest()
{
    var iCount = 30000;
    var testString = "one two     three four   five six    seven eight   nine  ten eleven " +
        "twelve  thirteen  fourteen fifteen sixteen seventeen   eighteen " +
        "nineteen                twenty";
    var testStrings = [testString, testString + " "];

    for (var i = 0; i < iCount; i++)
    {
        var matchString = testStrings[i % 2];
        var destArray = matchString.split(/\s+/);
    }
}

// BEGIN POSTLUDE
RunTest();
var _interval = new Date() - _startDate;

echo("### TIME: " + _interval + " ms");
// END POSTLUDE