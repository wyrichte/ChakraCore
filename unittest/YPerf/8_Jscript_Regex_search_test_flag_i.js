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
    var count = 50000;
    var testString = "There are many different ways to go about performance testing enterprise applications, some of them 		more difficult than others. The type of performance testing you will do depends on what type of results you want to 	achieve. For example, for repeatability, benchmark testing is the best methodology. However, to test the upper 	limits of the system from the perspective of concurrent user load, capacity planning tests should be used. This 	article discusses the differences and examines various ways to go about setting up and running these performance 	tests.\nThe key to benchmark testing is to have consistently reproducible results. Results that are reproducible 	allow you to do two things: reduce the number of times you have to rerun those tests; and gain confidence in the 	product you are testing and the numbers you produce. The performance-testing tool you use can have a great impact 	on your test results. Assuming two of the metrics you are benchmarking are the response time of the server and the 	throughput of the server, these are affected by how much load is put onto the server.\nThe amount of load that is 	put onto the server can come from two different areas: the number of connections (or virtual users) that are 	hitting the server simultaneously; and the amount of think-time each virtual user has between requests to the 	server. Obviously, the more users hitting the server, the more load will be generated. Also, the shorter the think	-time between requests from each user, the greater the load will be on the server. Combine those two attributes in 	various ways to come up with different levels of server load. Keep in mind that as you put more load on the server, 	the throughput will climb, to a POINT."
    var testStrings = [testString, testString + " "];
    var regex = /POINT/i;

    for (var i = 0; i < count; i++)
    {
        var matchString = testStrings[i % 2];
        regex.test(matchString);
    }
}

// BEGIN POSTLUDE
RunTest();
var _interval = new Date() - _startDate;

echo("### TIME: " + _interval + " ms");
// END POSTLUDE