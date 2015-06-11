var deepfunction = function() {
  function f1() 
  {
    function f2() 
    {
      function f3() 
      {
        var x = 1; /**bp:stack()**/
        return x;
      }
      f3();
    }
    f2();
  }
  f1();
}

deepfunction();
WScript.Echo("PASS");
