/**exception(resume_ignore):stack();locals();**/
var abc = [];
abc.sort();
var a = 0;
var c = 0;
function bar() {
  foo();
  foo();
  c++;
  if (c == 3) {
    WScript.Echo("pass");
  }
}
function foo() {
  a++;
  abc.sort(a > 1 ? null : undefined);
  c++; // Should get executed
}
WScript.Attach(bar);
