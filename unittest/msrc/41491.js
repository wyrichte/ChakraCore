var arr = Array();
function test0() {
  var func0 = function (regex) {
    for(let testcode = 0; testcode < 65536; testcode++) {
      const matches = ("a" + String.fromCharCode(testcode)).match(regex);
      if(matches != null) {
          arr.push(testcode);
      }
    }
  }
  var func1 = function () {
    func0(/^(a\n|ab)$/i);
  };
  func1();
}
test0();
if(arr.toString() == "10,66,98")
{
    console.log("PASSED");
}
else
{
    console.log(arr);
    console.log("FAILED");
}
