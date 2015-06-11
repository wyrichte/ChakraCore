// Test RegEx literal
var $pl1$ = 3
var $pl2$
var $pl3$=$pl2$ + 5;

// Test RegEx Object
$pl1$ = new RegEx("/d+/", "g")
$pl2$   = new RegEx("/[a-z]{4}$/")
$pl3$	=		new RegEX("/^[A-z]{4, 6}/")
$pl4$ =		new RegEx("/([a-z]|(A-Z)).*/", "gi")

// Test for Gloabl RegEx Property
$pl1$ = RegEx.index
$pl2$ = RegEx.	input
$pl3$		=	RegEx.$_
$pl6$=RegEx.$1