var ccc = x => x + 1;
var k = ccc(10); /**bp:stack();resume('step_into');stack();locals();resume('step_into');stack();locals();resume('step_into'); **/

function foo() {
  k = ccc(10); /**bp:stack();resume('step_into');stack();locals(); **/
  
  var nested = () => {
    return x => ccc(x); /**bp:stack();resume('step_into');stack();locals(); **/
  }
  
  k = nested()(10); /**bp:stack();resume('step_into');stack();locals(); **/
}

foo();

WScript.Echo(k === 11 ? "PASS" : "FAIL");
