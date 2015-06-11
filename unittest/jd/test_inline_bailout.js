// Test for inline scenarios with bailout
// Note: this is supposed to be called with -maxInterpretCount:1

var isUsingProfile = false;

function Dummy() {}

function foo3(sin)
{
  var x = sin(0);
  if (isUsingProfile) Debug.sourceDebugBreak();  	
}

function foo2() {
  // Cause bailout on inline built-in by passing Dummy rather than sin 
  // which was passed when we generated dynamic profile.
  var sin = isUsingProfile ? Dummy : Math.sin;
  foo3(sin);
}

function foo1() {
  foo2();
}

foo1(); // Genetate profile.
isUsingProfile = true;
foo1(); // Use profile.
