function opt(a, b, always_true = true) {
  a[0] = 1234;
  b[0] = 0;

  let arr = a;
  if (always_true) {
    arr = b;
    for (let i = 0; i < arr.length; i++)
      arr[i] = 0;
  }

  let val = arr[0];
  if (val) {
    print(val); // Must be 0, but prints out 1234
    return true;
  }

  return false;
}

let a = new Uint32Array(1);
let b = new Uint32Array(0x1000);
let failed = false;
for (let i = 0; i < 1000 && !failed; i++) {
  failed = opt(a, b);
}
print(failed ? "FAILED" : "PASS");
