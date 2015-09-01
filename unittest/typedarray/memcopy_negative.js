// Compares the value set by interpreter with the jitted code
// need to run with -mic:1 -off:simplejit -off:JITLoopBody
// Run locally with -trace:memop -trace:bailout to help find bugs

const Type = Int32Array;
const n = 100;

function test0(a, b) { for (let i = -50 , j = 0   ; i < n     ; i++, j++) b[i] = a[j]; }
function test1(a, b) { for (let i = -150, j = 0   ; i < n     ; i++, j++) b[i] = a[j]; }
function test2(a, b) { for (let i = 0   , j = -50 ; i < n     ; i++, j++) b[i] = a[j]; }
function test3(a, b) { for (let i = 0   , j = -150; i < n     ; i++, j++) b[i] = a[j]; }
function test4(a, b) { for (let i = -50 , j = -50 ; i < n     ; i++, j++) b[i] = a[j]; }
function test5(a, b) { for (let i = -100, j = -50 ; i < n     ; i++, j++) b[i] = a[j]; }
function test6(a, b) { for (let i = -50 , j = 50  ; i < n     ; i++, j++) b[i] = a[j]; }
function test7(a, b) { for (let i = -50 , j = 50  ; i < n + 50; i++, j++) b[i] = a[j]; }
function test8(a, b) { for (let i = 50  , j = -50 ; i < n + 50; i++, j++) b[i] = a[j]; }

const testCases = [
  test0,
  test1,
  test2,
  test3,
  test4,
  test5,
  test6,
  test7,
  test8
];

function fill(a) { for (let i = 0; i < n; i++) a[i] = i; }
let passed = 1;

for(let fnTest of testCases) {
  let src = new Type(n);
  fill(src);
  let interpreterCopy = new Type(n);
  let JitCopy = new Type(n);
  fnTest(src, interpreterCopy);
  fnTest(src, JitCopy);
  for(let j = 0; j < n; ++j) {
    if(interpreterCopy[j] !== JitCopy[j])
    {
      passed = 0;
      WScript.Echo(fnTest.name + " " + j + " " + interpreterCopy[j] + " " + JitCopy[j]);
      break;
    }
  }
}

if(passed === 1) {
  WScript.Echo("PASSED");
} else {
  WScript.Echo("FAILED");
}
