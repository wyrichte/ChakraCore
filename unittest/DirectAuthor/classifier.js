// Scenario to test JScript Inbuilt Method - eval
var $pl1$ = eval(6*3)

// Scenarito to Test JScript Inbuilt functions
$pl1$ = ScriptEngineMajorVersion();
$pl2$=SciptEngineMinorVersion();

// Test for JScript Intrisic object - string()
$pl1$ = new String("#pl2#")
$pl2$ = $pl1$.length
$pl1$. test = 10
$pl1$	.	blink()

// Test for JScript Intrinsic object - Number()
var $pl1$=new Number	() ;
$pl1$.max = Number.MAX_VALUE;
$pl1$.min = Number.MIN_VALUE;
$pl2$ = $pl1$.toExponential(15)

// Test for JScript Intrinsic object - Arguments()
function $pl1$ ($pl2$, $pl3$)
{
	var $pl4$ = arguments.length;
	var $pl5$ = $pl1$.length;
}

// Test for JScript Intrinsic object - Math()
$pl1$ = Math.PI;
$pl2$ = Math.acos(-14)
$pl3$ = Math.acos($pl2$)

for (var of of of) { }
for (let of of of) { }
for (const of of of) { }
for (of of of) { }

for (var /*comments should not*/ of /*affect the pattern*/ of /*matching of contextual keywords*/of) { }
for // comment
(of // comment
 of of) { }

o./*asdf*/function = 0; // function should be an identifier here, not a keyword
var basic = `this is a string template without any expression in it`;
var strExpr1 = `Fifteen is ${a + b} and not ${2 * a + b}.`;
var strExpr2 = `he${"l"+"l"}o ${'w' +  `o${"rl"}d`} !`

var strExpr3 = `hello
world` // comment

var strExpr4 = `hello ${
"world"} !`

var strExpr5 = `hello ${"world"
} ` + `${'!'}`
