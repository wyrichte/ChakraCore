// Test operators in simple expressions
var $pl1$ = 3
var $pl2$
var $pl3$=$pl2$ + 5;

// Operators in expressions with multiple sub expressions
var $pl1$=555
var $pl2$	=	($pl1$		+		3)-6   +  888

// Operators in Expression spanning multiple lines
var $pl1$   = (5++---3*(++2%56)
+8)/4
var $pl1$   = (5++ -	--3	* (++2 % 56)
			+8)/4
		- 234

// Operators in Expressions consisting of identifiers and constants
res=num1+55-num2*87

// Operators in Hetrogenous expressions
$pl1$ = "Hello World"+3+"times";
$pl2$ = "Bye World"+3+"times"
$pl3$ = "Hello Again"+3+"times";

// Test for Arthimetic operator
var $pl1$ = +((((++$pl2$ - $pl3$++ + --$pl4$) * 10) % 200) / 1024)
var $pl5$ = -($pl6$--) + "#pl7#"

// Test for Relational Operators
for ($pl1$=0, $pl2$=10;
	$pl1$<=10, $pl2$>=0;
		$pl1$+=1, $pl2$-=1)
{
	if ($pl3$ + $pl2$ < 90)
		continue;
	else if ($pl3$ + $pl2$ > 200)
		break;
}

// Test for Equality operator
if (($pl1$ == $pl2$) || ($pl1$ === $pl2$))
	$pl3$=true;

// Test for Inequality operator
if (($pl1$ != $pl2$) && ($pl1$ !== $pl2$))
	$pl3$	=	false

// Test for Binary Operators
$pl1$ = ($pl2$ << $pl3$) | ($pl4$ >> $pl5$)
$pl6$ = ~(($pl7$ >>> $pl8$) & ($pl9$ ^ $pl10$))

// Test for Ternary Operators
var $pl1$, $pl2$
$pl2$ = ($pl1$>10)?100:1000

// Opertors as part of args passed to functions
function $pl3$ ($pl1$, $pl2$)
{
	return $pl1$ + $pl2$
}
$pl3$ ($pl4$++, ($pl5$--)*4)

// Hetrogenous Expressions
$pl1$ = new Date()

$pl2$ = $pl1$.getDate() + "/"
		+ ($pl1$.getMonth()+1)

		+ "/" + $pl1$.getYear();