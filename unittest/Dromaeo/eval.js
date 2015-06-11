function write(s) {     
    if (typeof(WScript) == "undefined")
        document.write(s + "<br/>"); 
    else 
        WScript.Echo(s); 
}

function log(v) {
    // uncomment next line if individual results required
    // write(v);
}

var testScenario = "";

var startDate;

function startTest(scenario, testId)
{
    startDate = new Date();
	testScenario = scenario;
	log("Testing Scenario : " + testScenario);
}

function prep(fn) {
    fn();
}

function test(testName, fn) {
	var s1 = new Date();
	
	fn();
	
	log(testName + ": " + (new Date() - s1) );
}

function endTest() {
	log(testScenario + " finished");
	write("### TIME: " + (new Date() - startDate) + " ms");
}

(function(){ startTest("dromaeo-core-eval", 'efec1da2');

// Try to force real results
var ret, tmp;

// The commands that we'll be evaling
var cmd = 'var str="";for(var i=0;i<1000;i++){str += "a";}ret = str;';

// TESTS: eval()
var num = 4;

prep(function(){
	tmp = cmd;

	for ( var n = 0; n < num; n++ )
		tmp += tmp;
});

test( "Normal eval", function(){
	eval(tmp);
});

test( "new Function", function(){
	(new Function(tmp))();
});

endTest(); 
}) ();
