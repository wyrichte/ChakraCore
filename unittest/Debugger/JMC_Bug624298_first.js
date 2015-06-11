// Validate that on library code we break as a correct break-reason.

WScript.LoadScriptFile('JMC_Bug624298_second.js');

function Print(str) {
    WScript.Echo(str);
};

Print('Start!');
test1(); /**bp:dumpBreak();resume('step_into');dumpBreak();resume('step_document');dumpBreak();**/
Print('test1 Completed!');
test2();/**bp:dumpBreak();resume('step_into');dumpBreak();resume('step_document');dumpBreak();**/
Print('test2 Completed!');
