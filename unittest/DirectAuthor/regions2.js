/* ##pl1##  - Scenario to Test Single line C style comments */
function add( $pl1$ , $pl2$)
{
	return $pl1$ + $pl2$;
}

function newFunc() {


/*
 * Scenario to test Multi line C style comment
 * function to multiply *
 * two values		*/
function multiply( $pl1$ , $pl2$)
{
	return $pl1$ * $pl2$;
}

// Scenario to test single line c++ comment
function add( $pl1$ , $pl2$)
{
	return $pl1$ + $pl2$;
}

// Scenario to test multi line c++ comment
// function to add two values
function add( $pl1$ , $pl2$)
{
	return $pl1$ + $pl2$;
}

// Scenario to Test c++ line comment at the end
var $pl1$ = 10; //##pl2##

/*
//##pl6## C, C++ style comments interleaved
*/
var a, b

/*	##pl1## */
//##pl2## C style comment followed by C++ style comment
var $pl3$

//
// ##pl1## - C++ Style Comments outside all blocks
//		##pl6##
//
var $pl2$, $pl3$;	// ##pl5##
var $pl4$

// C++ Style Comments within functional block
function $pl1$ ($pl2$, $pl3$)
{
	//
	// ##pl4##
	//
	var $pl4$

	//	##pl6##
	return $pl2$ + $pl3$;	// ##pl5##
}

// / operator followed by C++ style comments
var j = 10/10; //comment