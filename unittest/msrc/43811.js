function func1() {
  let stack_number = (1.1 + 1);
  return stack_number["x"]; // LdFld
}
func1();
Number.prototype.__defineGetter__("x", Object.prototype.valueOf);
var x1 = func1();

function func2(arg) {
  let stack_number = (1.1 + 1);
  return stack_number[arg]; // LdElemI_A
}
func2();
Number.prototype.__defineGetter__("y", Object.prototype.valueOf);
var x2 = func2("y");

WScript.Echo("pass");
