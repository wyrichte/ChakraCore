
var a = 1;

// ------------------------------------------------
// Positive cases. All of these should return a completion list.
// ------------------------------------------------
a.!I:;
a.to!I:Expotent;
a.toExpotent!I:;
a.to!I:Expotential();
/* some comment */ a.!I:
a.!I: // some comment
;
// ------------------------------------------------
// Negative cases. These should not return symbols
// ------------------------------------------------

// Statement scope
!I:

// Statement scope in a function
function() {
  !I:
}

// After an integer.
5.!I:;

// After a double
5.3.!I: ;

// In single line comment.
a. // .!I:
;

// In a multi line comment.
a. /* 

  .!I: 

*/
;
// In strings
"unterminated string.!I:
"terminated string.!I:"
'unterminated single quote.!I:
'terminated single quote.!I:'

// In regular expressions
//  terminated
var r1 = /.!I:*/;
//  unterminated
var r1 = /abc.!I:
