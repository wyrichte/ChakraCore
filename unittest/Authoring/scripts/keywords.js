// Test var keyword in global scope
var $pl1$ = 3
  var  $pl2$ = 5;
		var	$pl3$ = 6;

// Test var keyword in local scope
function ()
{
var $pl1$ = 2
   var $pl2$=3;
		var		$pl3$=6;
}

// Keywords followed by other keywords
for(var $pl1$=10; $pl1$>0; $pl1$--)
{
	$pl2$++;
}

// keywords in nested scope
var $pl1$=10
var  $pl2$=20
var	$pl3$

for ($pl1$; $pl1$>5; --$pl1$)
{for($pl2$; $pl2$>10; $pl2$-=1)
  {
	$pl3$ += $pl2$ + $pl1$
  }
}

// Test for do-while keyword
do{
$pl1$ += 10;
while ($pl2$-- > 5)

// Test for switch-case-default keyword
switch ($pl1$)
{
	case 'a':
		$pl2$ = "short"
		break;
	case 'A' :
		$pl2$ = "long"
		break;
	default	:
		$pl2$ = "unknown"
}

// Test for try-catch-throw-finally keyword
try
{
	$pl3$ = $pl1$ ($pl2$)
}catch(e){
	throw "#pl4#"
}finally{
	$pl5$ = false;
}

// Test for void-typeof keyword
$pl1$
void $pl2$ ($pl3$)
{
	if (typeof($pl3$) == "undefined")
		$pl1$ = "not defined"
	else
		$pl1$ = typeof $pl3$
}

// Test for in-instanceof keyword
if ($pl1$ instanceof Array)
{
	$pl2$ = "Array";
} else if ($pl1$ instanceof Date)
	$pl2$ = "Date"
} else {
	$pl2$ = "Object"
}

// Keywords passed as arguments to functions
function $pl1$ ($pl2$, $pl3$, $pl4$)
{
	this.$pl5$ = $pl2$
	this.$pl6$ = $pl3$
	this.$pl7$ = $pl4$
}

$pl8$ = new $pl1$ ($pl7$, true, false)

// JScript specific Keywords - debugger
while ( $pl1$ > 10)
{if ($pl2$){break;}debugger}

// Future reserved keywords
int $pl1$
static int $pl2$
boolean $pl3$
const char $pl4$
double $pl5$
float $pl6$

// Keywords as part of identifier names
variable1
whilefor
int integer
var __for

// Keywords as identifiers
var continue, delete;
null; with;
break, continue;

// Keywords with case changed
Continue = "continue";
Try;
for (var Var=20; Var>0; Var++)
{
	if (typeof(arr[Var] == "undefined")
		eval (Continue)

	arr[Var] += Var;
	Try = true;
}