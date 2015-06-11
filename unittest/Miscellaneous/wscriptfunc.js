var func = Debug.parseFunction("WScript.Echo('hello')");
func();
var func1 = Debug.parseFunction('this.foo = function(one, two) {WScript.Echo(one + two)}; ');
func1();
foo('hello', 'world');
