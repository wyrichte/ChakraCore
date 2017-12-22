
var asmModule = new Function(`
"use asm";
  function f() {
    ${Array(100).fill("if(0){").join("\n")}
    ${Array(100).fill("}").join("\n")}
  }
  return f;
`);

function recur() {
  try {
    recur();
  } catch (e) {
    print("Ran out of stack");
    // Ran out of stack, now trigger asm.js linking failure
    asmModule(1);
  }
}
print("Starting recursion");
recur();
print("failed, expected a failfast");
