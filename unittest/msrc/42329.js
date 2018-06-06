function opt() {
  let arr = [];
  return arr['x'];
}

function main() {
  let arr = [1.1, 2.2, 3.3];
  opt();
  Array.prototype.__defineGetter__('x', Object.prototype.valueOf);
  var x = opt();
}
main();
WScript.Echo("pass");