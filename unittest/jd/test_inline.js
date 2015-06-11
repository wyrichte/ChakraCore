// Test for inline scenarios

function foo3()
{
  Debug.sourceDebugBreak();  	
}

function foo2() {
  foo3();
}

function foo1() {
  foo2();
}

foo1();
