// Validate returned value on the library code

WScript.LoadScriptFile('returnedvaluetests5_sub.js');

function Print(str) {
    WScript.Echo(str);
};
function callback() {
	return "callback";
}

Print('Start!');
test1(); /**bp:resume('step_into');dumpBreak();resume('step_document');dumpBreak();locals();**/
Print('test1 Completed!');
test2(); /**bp:resume('step_into');dumpBreak();resume('step_document');dumpBreak();resume('step_into');resume('step_into');resume('step_document');dumpBreak();locals();**/
Print('test2 Completed!');
