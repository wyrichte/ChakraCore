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
    var count = 10000;
    var testString = "a123456789b23456789c23456789d23456789e23456789"
    var testStrings = [testString, testString + " "];
    var regex = /(\w|\s){1}/g; 

    for (var i = 0; i < count; i++)
    {
        var matchString = testStrings[i % 2];
        matchString.match(regex);
    }
}

// BEGIN POSTLUDE
RunTest();
var _interval = new Date() - _startDate;

echo("### TIME: " + _interval + " ms");
// END POSTLUDE