// Make sure that in hybrid debugging when arguments are not provided, 
// we still create new/fake args object rather than saying 
function test0(){
  var obj0 = {};
  function bar0 (){
    obj0 = 1; /**bp:setFrame(1);locals();**/
  }
  bar0();
  var x = arguments[1];
};
test0();

WScript.Echo("done");
