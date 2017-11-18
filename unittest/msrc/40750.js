function opt(a, b, v) {
  if (b.length < 1) return;

  for (let i = 0; i < a.length; i++)
    a[i] = v;

  b[0] = 2.3023e-320;
}

for (let i = 0; i < 1000; i++) {
  opt(new Uint8Array(100), [1.1, 2.2, 3.3], {});
}

const a = new Uint8Array(100);
const b = [1.1, 2.2, 3.3];
opt(a, b, {
  valueOf: () => {
    b[0] = {};
    return 0;
  }
});
if (b[0] == 2.3023e-320) {
  print("pass");
}
