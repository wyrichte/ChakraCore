function detach(heap) {
  // TODO:: Change this to ArrayBuffer.detach in master branch
  ArrayBuffer.transfer(heap);
}
function trytest(fn) {
  try {
    fn();
    print("Should have had an exception");
  } catch (e) {
    // print("Successfully caught");
  }
}

function asmModule(stdlib, foreign, heap) {
  "use asm";
  var arr = new stdlib.Int32Array(heap);
  var ext = foreign.ext;

  function f() {
    arr[0] = ext(1) | 0;
    return arr[10] | 0;
  }
  function g(x) {
    x = x|0;
    return arr[10] | 0;
  }
  return {f:f, g:g};
}

const global = this;
function runExternalDetach(doJit) {
  let glob = 1;
  const heap = new ArrayBuffer(0x10000);
  const f = asmModule(global, {ext: () => ({valueOf: () => glob === 0 ? detach(heap) : 1})}, heap).f;
  if (doJit) {
    for (let x = 0; x < 10000; x++) f();
  }
  glob = 0;
  trytest(f);
}
runExternalDetach(false);
runExternalDetach(true);

function runArgumentDetach(doJit) {
  const heap = new ArrayBuffer(0x10000);
  const g = asmModule(global, {ext(){}}, heap).g;
  if (doJit) {
    for (let x = 0; x < 10000; x++) g();
  }
  trytest(() => {
    g({valueOf() {detach(heap);}});
  });
}
runArgumentDetach(false);
runArgumentDetach(true);
print("pass");
