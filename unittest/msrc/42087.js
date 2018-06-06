function go(a0) {
  var b = a0();
  return b.toString(16);
}

// <-- force JIT, always using a Function as an argument
for (let i = 0; i < 0x1000; i++) {
  go(String.fromCharCode);
}

// <-- this function will return an integer and not an object
function boom() {
  return 0x1000;
}

if (go(boom) === "1000") {
  print("pass");
}
