// Object Mutation Breakpoint
// Cross script context breakpoint

/**onmbp:locals(1);**/

var e = WScript.LoadScriptFile(".\\mutationBpTest3Load.js", "samethread");

function eChangeProp()
{
	e.changeProp();/**bp:mbp("[Globals].e.obj.prop", "value", 'update', "eprop");**/
}
eChangeProp();
e.changePropAgain(); /**bp:deleteMbp("eprop")**/
WScript.Echo("pass");
