/*
    Simple attach-detach scenario
*/


/**exception(resume_ignore):locals();stack()**/

function Run() {
    var x = 100;
    x; /**bp:evaluate('x|o')**/
	WScript.Echo('PASSED');
}

WScript.Attach(Run);
WScript.Detach(Run);

