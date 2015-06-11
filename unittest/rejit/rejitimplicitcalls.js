function thing() { 
  this.isA = "foo";
}

function test(thing, bail) { 
  var isA; 
  
  for (var i = 0; i < 20; i++) {
    if (!(bail == undefined)) {
      isA = 1.1 * bail;
    }
    else if (i % 5 == 0) {
      isA = thing.isA;    
    }
  }
  
  return isA;
}

t = new thing();

thing.prototype = {
  get isA() {
    return "bar";
  }
}

t2 = new thing();

// JIT the loop body with no implicit calls.
// Induce an instrumented JIT by causing non-implicit calls bailout.
WScript.Echo(test(t, "bail"));
// Induce implicit calls, bailout and rejit the loop.
WScript.Echo(test(t2));
// Once more time so test gets JITed
WScript.Echo(test(t));
// We should not bailout from test
WScript.Echo(test(t2));




