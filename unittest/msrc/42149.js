try {
  (function() {
    "use asm";
    function f() {
      arr();
    }
    function g() {}
    var arr = [g];
    return f;
  })()();
} catch (e) {
  if (!(e instanceof TypeError)) {
    throw e;
  }
}
