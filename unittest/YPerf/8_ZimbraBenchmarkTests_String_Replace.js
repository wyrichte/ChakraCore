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
    var iCount = 200000;
    var testString = "<html><head><meta http-equiv=\"X-UA-Compatible\" content=\"IE=8\" /><title>Benchmark tests from Zimbra.</title></head><body></body></html>";
    var testStrings = [testString, testString + " "];
    var regex = /</g;
    var replace = "&lt;";

    for (var i = 0; i < iCount; i++)
    {
        var matchString = testStrings[i % 2];
        var destString = matchString.replace(regex, replace);
    }
}

// BEGIN POSTLUDE
RunTest();
var _interval = new Date() - _startDate;

echo("### TIME: " + _interval + " ms");
// END POSTLUDE