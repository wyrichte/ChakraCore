/**exception(resume_ignore):stack();locals();**/
var a = 0;
var b = 0;
var isInJitAvailable = (typeof Debug == 'object') && (typeof Debug.isInJit == 'function');
function bar() {
  foo();
  foo();
  b++;
  if (b == 3) {
    WScript.Echo("pass");
  }
}

function foo() {
  a++;
  if (isInJitAvailable) {
    var isInJit = Debug.isInJit();
    if (a > 1 && !isInJit) {
      WScript.Echo("Failed");
    }
  }
  eval("");
  new Function();
  (new(function  * () {}).constructor("return 1")()).next();
  new(async function () {}).constructor("return 1")()
  b++; // Should get executed
}
WScript.Attach(bar);
