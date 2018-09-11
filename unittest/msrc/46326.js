function foo() {
    var bar = function () {
      return new.target;
    };
    Reflect.construct(bar, [])();
  }
  
  foo();
  foo();
  foo();
  
  WScript.Echo("pass");
  