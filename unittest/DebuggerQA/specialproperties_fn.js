function test(a, b, c) {
	arguments;/**bp:evaluate('arguments', 1)**/
    arguments;/**bp:evaluate('arguments.callee', 1)**/
	arguments;/**bp:evaluate('arguments.callee.caller', 1)**/
}
function bar(){	
	test(1, 2, 3);
}
var arrow = () => {};
bar(); 
WScript.Echo('Pass');/**bp:evaluate('arrow',1)**/


