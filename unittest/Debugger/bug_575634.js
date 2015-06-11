var foo = function() {
  var x; /**bp:stack()**/
}

var o = {
  bar: function() {
    this; /**bp:stack()**/
    return 0;
  }
};

foo();
o.bar();

WScript.Echo("PASS");
