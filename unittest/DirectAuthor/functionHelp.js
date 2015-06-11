//
// Nested method calls - 3 levels
//
function ParamIndexTest() {
	function fnc(aa, bb, cc, dd) {}
	fnc(0, 1, 2,|
}

//
// Nested method calls - 3 levels
//
function NestedMethod3() {
	var testobj = { testobjmethod1: function(param1) { } }
	testobj.testobjmethod1(testobj.testobjmethod1(testobj.testobjmethod1(|
}

//
// Calling a method, one of the arguments is a method call result
//
function MethodCallWithMethodCallArg() {
	var testobj = { testobjmethod1: function(param1) { } }
	testobj.testobjmethod1(|, testobj.testobjmethod1())
}

//
// Completion in nested method call param
//
function NestedMethodCallParam() {
	var testobj = { testobjmethod1: function(param1) { } }
	testobj.testobjmethod1(2, testobj.testobjmethod1(|
}

//
// Nested call expressions - methods
//
function NestedCallExpressionsTest_Methods()
{
	var obj = { m1:function(a) { }, m2: function(b) { } };
	function f1(a) {}
	function f2(a) {}
	obj.m1(obj.m2(|));
	obj.m2(obj.m1(),|);
}

//
// Calling a method via identifier
//

var objid = { method10: function(a, b, c) { } };
var methodid = objid.method10;
methodid(|);

//
// DoWhileBlock
//
function DoWhileBlock() {
	function doWork(w) { }
	var c = 0;
	do 
	{
		doWork(|);
	}
	while(c);
}

//
// Return anonymous function
//
function ReturnAnonymousFunction() {
	function a() {
		return function(bb,cc) { }
	}
	a()(|
}

//
// BeforeComment
//
function BeforeComment() {
	function doWork(w) { }
	doWork(| /* comment */
}

//
// InsideIf
//
function InsideIf() {
	function doWork(w) { }
	if(doWork(|) {
	}
}

//
// Anonymous function
//
function AnonymousTest() {
	var f;
	f = function(a, b) { }
	f(|
}

//
// InWhileBlock
//
function InWhileBlock() {
	function doWork(w) { }
	var c = 1;
	while(c) {
		doWork(|
	}
}

//
// Constructor (new)
//
function ctorTest() {
	function viaCtor(cparam) { }
	var newVal = new viaCtor(|;
}

//
// Calling outer function from a nested function (2 levels)	
//
function outer(c, d) {
	function inner1(a1, b1) {
		outer(|
	}
}

//
// Calling a nested function	
//
function NestedFunctionTest() {
	function nested1(param1, param2) { }
	nested1(|
}

//
// Nested function via identifier	
//
function NestedFunctionsTest() {
	function outer(c, d) {
		function inner1(a1, b1) { }
		var inner1Var = inner1;
		inner1Var(|
	}
}

//
// Nested call expressions - functions
//
function NestedCallExpressionsTest_Funcs()
{
	function f1(a) { }
	function f2(a) { }
	f1(f2(|
	f1(f2(|)
	f1(f2(|))
	f1(f2(),|;
}

//
// A global function definition
//
function f(a, b) { }
f(|);

//
// Missing closing parenthesis
//
function missingClosingParenthesis() { 
	function func(parameterName) { }
	func(|
}

//
// Object methods
//
function f1(a, b) { }
var obj = {
	method1: f1,
	method2: function(x, y) { },
	method3: function() { },
	method4: function(e) { return 1/0; } 
};

// Calling methods
obj.method1(|);
obj.method2(|);
obj.method3(|);
obj.method4(|);

//
// Calling methods inside a function
//
var globalObj1 = { method1: function(a, b, c, d)  {} };
function callingMethodsTest() {
	globalObj1.method1(|);
	var localObj = globalObj1;
	localObj.method1(|);
}

//
//	Errors in code path 
//
function f12()
{
	var localobj = { m: function(a, b) { } };
	localobj.NonExistentMethod();
	localobj.m(|);
}

function doWork(w) { }
function LastStatementInBlockTest() {
    if(true)
    {
        do
        { 
            doWork(|); 
        } while(false);
    }
}

//
//  Calling a field, the resulting function name should be the field name.
//
function CallFieldTest() {
    function global23(x)
    {
        return x;
    }
    var el = {
    field1: global23                
    };
    el.field1(|
}

//
// Calling via identifier, the resulting function name should be the identifier name.
//
function CallVarTest() {
    function shouldNotAppear(a) {}
    var shouldAppear = shouldNotAppear;
    shouldAppear(|
}

//
// A nested function calling another nested function on the same level	
//
function SameLevelNestedFunctionsTest() {
	function nested1(param1, param2) { }
	function nested2() { 
		nested1(|
	}
}

//
// Calling outer function from a nested function (3 levels)	
//
function outer1(o11, o12) {
	function outer2(o21, o22) {
		function outer3(o31, o32) {
			outer2(|
		}
	}
}

//
//  new inside for
//
function newInForTest() {
    function foo(id, name, str) { }
    for(var o = 1; o< 10; o++) {
	    var abc = new foo /* this is my comment */ (500,|);
    }
}

//
//  Prototype access in constructor method
//
function prototypeTest() {
    function o(a, b, c) {
	    this.superclass(|
    }
    o.prototype.superclass = function p(p1, p2) { };
}

//
// KNOWN TO FAIL, BUT NEED TO PASS EVENTUALLY
//

//
// Call immediately following declaration - inside function
//
function DeclareAndCallTest() {
	(function(a, b) { })(|)
}

//
// Call immediately following declaration - global
//
(function(a, b) { })(|)

//
// Calling library functions
//
function LibraryCallTest() {
	var n = new Number(1);
	var s = n.toString(|
}

//
// NEGATIVE TESTS - EMPTY RESULT EXPECTED FOR THE TESTS BELOW
//

//
// Calling this method
//
var objThis = {
	method1: function(p1, p2) {
	},
	method3: function() { 
		this.method1(|); 
	}
};

//
// Non existent object methods
//
function nonExistentMethodTest() {
	var obj = { };
	obj.method0(|);
	obj.method1(|);
}

//
// Invalid location
//
function InvalidLocation() {
	if(|
}

//
//  Int result call
//
function intCallTest() {
    function test(a, b, c) {
        return a + b + c;
    }
    test(5, 6, 7)(|
}



