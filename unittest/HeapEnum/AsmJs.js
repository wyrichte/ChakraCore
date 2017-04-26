function AsmModule(env) {
  "use asm";
  function func(x) {
    x = x | 0;
    return x | 0;
  }
  return func
}
var asmModule = AsmModule({});
Debug.dumpHeap(asmModule, true);