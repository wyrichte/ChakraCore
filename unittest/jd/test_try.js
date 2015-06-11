// test case for try-catch and try-finally to test amd64 EH frames scenario.

function foo2() {
  Debug.sourceDebugBreak();
}

function foo1() {
  try {
    foo2();
  } catch (ex) {
  }
}

function bar2() {
  Debug.sourceDebugBreak();
}

function bar1() {
  try {
    bar2();
  } finally {
  }
}

foo1();
bar1();
