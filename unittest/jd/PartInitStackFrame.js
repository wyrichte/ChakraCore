// -nonative -InjectExceptionAtPartiallyInitializedInterpreterFrame:3

// We only check interpreter frame which code addr matches one from frames pushed to scriptContext.
// Thus use same function body (causes same interpreter thunk).

function createFoo()
{
  var foo = function(another)
  {
    if (another) another();
  }
  return foo;
}

var foo1 = createFoo();
var foo2 = createFoo();
foo1(foo2);
