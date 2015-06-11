// Identifiers declared at global scope
var x_coordinate
var imaginary_i_
var __internal
var tmp123

y_coordinate
imaginary_j_
tmp234

// Identifiers delared at local scope
function $pl1$ ()
{
var x_coordinate
var imaginary_i_
	var __internal
	var tmp123

y_coordinate
imaginary_j_
	tmp234
}

// Object Names as Identifiers
function $pl1$ ($pl2$,
			$pl3$,
				$pl4$)
{
	this.$pl5$ = $pl2$
	this.$pl6$ = $pl3$
	this.$pl7$ = $pl4$
}

$pl9$ = new $pl1$ ($pl8$, $pl9$, "#pl10#")

// Array names as Identifiers
$pl1$ = new Array()

$pl2$ = $pl1$ [$pl2$]
$pl3$ = $pl1$[0]
$pl4$ = $pl1$ [ $pl5$ ]

// Host Objects As Identifiers
document.write("Hello World- how are you");
WScript.Echo("Goodbye world");

// Labels As Identifiers
outerloop:
for ($pl1$ = 10; $pl1$>0; $pl1$--)
{
	innerloop	:
	for ($pl2$ = 100; $pl2$>20; --$pl2$)
	{
		if ($pl1$ == 5)
			break innerloop
		if ($pl2$ == 25)
			break outerloop

		$pl3$ = $pl1$ + $pl2$
	}
}