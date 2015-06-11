// String constants assinged to variables - gloabal scope
var $pl1$="#pl2#"
var $pl3$  =  "#pl4#";
var $pl5$	=		"#pl5#"

// String literals assigned to variables without var keyword - Global scope
$pl1$="#pl2#";
$pl3$  =     "#pl4#"	
$pl5$	=	"#pl6#";

$pl7$="#pl8#"

// String constants assigned to variables - local scope
function ()
{
	var str1="Hi"
	var str2  =  "Bye"
	var str3	=		"Hello"
}

// Unterminated String literals ending with \
$pl1$="This is test \
   case"

// Special characters as part of strings
var $pl1$ = "This line has a new line character \n embedded within"
var $pl2$ = "This line has a escaped \\n within";

// Accessing Associative Object Members using strings
function MyObject (var1, var2, var3)
{
	this.var4 = var1;
	this.var5 = var2;
	this.var6 = var3;
}

myobj = new MyObject ("Hello", " World", " Once")
str = myobj["var4"] + myobj["var5"] + myobj.var6

// Strings as operands
$pl1$ = "#pl1#" + " " + "123" +""
$pl2$ = '#pl3#' + ' ' + '2345.34'

// String passed as args to function
$pl1$ = new Function("#pl2#", "#pl3#", "return #pl2# + #pl3#")

$pl1$ ("#pl4#", "#pl5#")

// Strings with escape sequences embedded within
$pl1$ = "This string contains \v vertical tab"
$pl2$ = 'This string contains \t horizantal tab'
$pl3$ = "String contains escp \"double quotes\""
$pl4$ = 'String Contains escp \'Single Quotes\''

// Nested Strings
$pl1$ = "This string 'has nested' single quote strings"
$pl2$ = 'This string "has nested double" quote strings'

// Operators as string
var $pl1$ = "((2 + 3) * 8++) / 23)"
$pl2$ = eval ($pl1$)

// keyowrds as strings
var $pl1$ = "looping keywords - for, while, do-while"
var $pl2$ = "Conditional check keywords - if, switch case default"