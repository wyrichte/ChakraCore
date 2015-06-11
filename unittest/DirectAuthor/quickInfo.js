
//
//  this
//
function Point(x, y) {
    this.x = x;
    th|is.y = y;
}
new Point(1, 1);

//
//  Local object property
//
function PropertyTest() {
    var x = { localNumProp: 2 };
    x.localNum|Prop;
}

//
//  Local array
//
function LocalArrayTest() {
    var arr = [ "a", "b"];
    ar|r;
}

//
//  Local function
//
function localFuncTest() {
    function localFunc(v) {}
    local|Func(2);
}

//
// Global object with doc comments
//
var j = {
    size: function () {
        ///	<summary>The number of elements currently matched.</summary>
        ///	<returns type="Number" />
        return this.length;
    },
    toArray: function() {
		///	<summary>Retrieve all the DOM elements contained in the jQuery set, as an array.</summary>
		///	<returns type="Array" />
		return slice.call( this, 0 );
	}
};

j.si|ze();
j.to|Array();

//
// Global Number value
//
var num = 2;
var num2 = n|um;

//
// Local Number value
//
function testLocal()
{
    var localnum = 2;
    var num2 = localn|um;
}

//
// Local argument 
//
function testLocalParam(a, localarg)
{
    var num2 = l|ocalarg;
}

//
// Number identifier inside a call
//
function insideCallArgsTest() {
    function calcRoot(v) {}
    var localnum = 2;
    calcRoot(localnu|m);
}

//
// Local object
//
function localObjectTest() {
	var localObj = { x: 10, y: 20 }
	moveUp(local|Obj, 1);	
}

//
//  Parameter of a Function type
//
function funcParameterTest(fnc) { f|nc(); }
funcParameterTest(function(a, b) { });

//
//  Parameter of undetermined type
//
function undeterminedParameterTest(undetermined) { undeter|mined(); }

//
// Closure of global variable
//
var globalClosureVar = "";
function closureOfLocalVarTest() {
    globalClosure|Var;
}

//
// Closure of local variable
//
function closureOfLocalVarTest() {
    var closureVar = "";
    function inner() {
        closure|Var;
    }
}

//
//
// Closure of global variable in inner func
//
var globalClosureVarInner = "";
function closureOfLocalVarTest() {
    function inner() {
        globalClosureVar|Inner;
    }
}


//
//  CASES WHICH ARE CURRENTLY NOT SUPPORTED BUT MAY BE SUPPORTED IN THE FUTURE


//
//  On argument
//
function OnArgumentTest(ar|g) {
}

//
//  On var
//
function OnVarTest() {
    var x|x = { }
}

//
// NEGATIVE TESTS
//

//
//  On keyword
//
function OnKeywordTest() {
    i|f(true) {}
}


