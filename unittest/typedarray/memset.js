// Compares the value set by interpreter with the jitted code
// need to run with -mic:1 -off:simplejit -off:JITLoopBody
// Run locally with -trace:memop -trace:bailout to help find bugs

const global = this;
const types = "Int8Array Uint8Array Int16Array Uint16Array Int32Array Uint32Array Float32Array Float64Array".split(" ");
const n = 300;

function test()
{
  const i1 = 0, i2 = 5, i3 = -7, f1 = 3.14, f2 = -9.54715685478;
  const j1 = -50, j2 = 51, j3 = 100, j4 = 200, j5 = 275;
  let i;
  const a1 = new global[types[0]](n);
  for(i = j1; i <= j2 - 1; i++)    { a1[i] = i1; }
  for(i = j3; i < j4; i++)         { a1[i] = i3; }
  i = j2; while(i < j3)            { a1[i] = i2; ++i; a1[i] = i2; ++i; }
  for(i = j5 - 1; i > j4 + 1; i--) { a1[i] = f1; }
  for(i = n - 1; i >= j5; i--)     { a1[i] = f2; }

  const a2 = new global[types[1]](n);
  for(i = j1; i <= j2 - 1; i++)    { a2[i] = i1; }
  for(i = j3; i < j4; i++)         { a2[i] = i3; }
  i = j2; while(i < j3)            { a2[i] = i2; ++i; a2[i] = i2; ++i; }
  for(i = j5 - 1; i > j4 + 1; i--) { a2[i] = f1; }
  for(i = n - 1; i >= j5; i--)     { a2[i] = f2; }

  const a3 = new global[types[2]](n);
  for(i = j1; i <= j2 - 1; i++)    { a3[i] = i1; }
  for(i = j3; i < j4; i++)         { a3[i] = i3; }
  i = j2; while(i < j3)            { a3[i] = i2; ++i; a3[i] = i2; ++i; }
  for(i = j5 - 1; i > j4 + 1; i--) { a3[i] = f1; }
  for(i = n - 1; i >= j5; i--)     { a3[i] = f2; }

  const a4 = new global[types[3]](n);
  for(i = j1; i <= j2 - 1; i++)    { a4[i] = i1; }
  for(i = j3; i < j4; i++)         { a4[i] = i3; }
  i = j2; while(i < j3)            { a4[i] = i2; ++i; a4[i] = i2; ++i; }
  for(i = j5 - 1; i > j4 + 1; i--) { a4[i] = f1; }
  for(i = n - 1; i >= j5; i--)     { a4[i] = f2; }

  const a5 = new global[types[4]](n);
  for(i = j1; i <= j2 - 1; i++)    { a5[i] = i1; }
  for(i = j3; i < j4; i++)         { a5[i] = i3; }
  i = j2; while(i < j3)            { a5[i] = i2; ++i; a5[i] = i2; ++i; }
  for(i = j5 - 1; i > j4 + 1; i--) { a5[i] = f1; }
  for(i = n - 1; i >= j5; i--)     { a5[i] = f2; }

  const a6 = new global[types[5]](n);
  for(i = j1; i <= j2 - 1; i++)    { a6[i] = i1; }
  for(i = j3; i < j4; i++)         { a6[i] = i3; }
  i = j2; while(i < j3)            { a6[i] = i2; ++i; a6[i] = i2; ++i; }
  for(i = j5 - 1; i > j4 + 1; i--) { a6[i] = f1; }
  for(i = n - 1; i >= j5; i--)     { a6[i] = f2; }

  const a7 = new global[types[6]](n);
  for(i = j1; i <= j2 - 1; i++)    { a7[i] = i1; }
  for(i = j3; i < j4; i++)         { a7[i] = i3; }
  i = j2; while(i < j3)            { a7[i] = i2; ++i; a7[i] = i2; ++i; }
  for(i = j5 - 1; i > j4 + 1; i--) { a7[i] = f1; }
  for(i = n - 1; i >= j5; i--)     { a7[i] = f2; }

  const a8 = new global[types[7]](n);
  for(i = j1; i <= j2 - 1; i++)    { a8[i] = i1; }
  for(i = j3; i < j4; i++)         { a8[i] = i3; }
  i = j2; while(i < j3)            { a8[i] = i2; ++i; a8[i] = i2; ++i; }
  for(i = j5 - 1; i > j4 + 1; i--) { a8[i] = f1; }
  for(i = n - 1; i >= j5; i--)     { a8[i] = f2; }

  return [a1, a2, a3, a4, a5, a6, a7, a8];
}

// Run first time in interpreter
const a = test();
// Run second time with memop
const b = test();

let passed = true;
for(let i = 0; i < a.length; i++) {
  const aa = a[i], bb = b[i];
  for(let j = 0; j < aa.n; j++)
  {
    if(aa[j] !== bb[j])
    {
      WScript.Echo(types[i] + " " + j + " " + aa[j] + " " + bb[j]);
      passed = false;
    }
  }
}

if(passed)
{
  WScript.Echo("PASSED");
}
else
{
  WScript.Echo("FAILED");
}
