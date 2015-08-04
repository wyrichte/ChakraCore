// Compares the value set by interpreter with the jitted code
// need to run with -mic:1 -off:simplejit -off:JITLoopBody
// Run locally with -trace:memop -trace:bailout to help find bugs

var n = 10000;
var myHeap = new ArrayBuffer(n >> 2);
var a = new Array(myHeap);
var heap = new ArrayBuffer(n >> 2);
var b = new Array(heap);

function test(start)
{
  for (var i = start; i < start + (n / 2); i++) {
    b[i] = a[i];
  }
}

function resetArrays() {
  for(let i = 0; i < n; i++)
  {
    a[i] = i;
    b[i] = 0;
  }
}

resetArrays();

var passed = 1;
function compareResults(start, end) {
  for(let i = start; i < end; i++)
  {
    if(a[i] !== b[i])
    {
      print(`Invalid value: a[${i}] != b[${i}]`);
      passed = 0;
      break;
    }
  }
}

test(0);
test(n / 2);
compareResults(0, n);
// test to help find bugs locally
// Expect this to bailout on BailOutOnFailedHoistedBoundCheck
// test(-50);

function test1()
{
  for (let i = -50; i < 10; i++) {
    b[i] = a[i];
  }
}
// run to jit the function
test1();
resetArrays();
// Expect this to bailout on MemOpError because of negative index
test1();
compareResults(-50, 10);
// test to help find bugs locally
// test1();
// Check for rejit
// test1();


if(passed === 1)
{
  WScript.Echo("PASSED");
}
else
{
  WScript.Echo("FAILED");
}
