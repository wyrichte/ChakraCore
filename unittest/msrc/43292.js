function foo1(a) {
  ({a = 1} = 1)
  arguments;
};

foo1();
foo1();

function foo2(a) {
  var c = 1;
  ({a = c} = 1)
  arguments;
};

foo2();
foo2();

function foo3(a, b) {
  ({a = b} = 1)
  arguments;
};

foo3();
foo3();

function foo4(a, b) {
  ({b = 1} = 1)
  arguments;
};

foo4();
foo4();

WScript.Echo("pass");