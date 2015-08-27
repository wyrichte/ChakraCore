test.verifyEqual(test.foo, undefined);
test.verifyEqual(test.bar, undefined);

test.foo = { foo : 1, bar : { } }
test.verifyEqual(test.foo.foo, 1);
test.verifyEqual(typeof(test.foo.bar), "object");

test.callback= (function(arg1, arg2) {
  test.foo = arg1;
  test.bar = arg2;
  test.verifyEqual(test.foo, "pi");
  test.verifyEqual(test.bar, 3.141592);
});

	