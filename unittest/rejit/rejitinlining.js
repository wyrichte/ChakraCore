function foo() {
  this.doSomething = function() {
    return { msg: "a foo did something" };
  }
  this.doSomethingElse = function() {
    return { msg: "a foo did something else" };
  }
  this.doAnotherThing = function() {
    return { msg: "a foo did another thing" };
  }
}

function bar() {
  this.doSomething = function() {
    return { msg: "a bar did something" };
  }
  this.doSomethingElse = function() {
    return { msg: "a bar did something else" };
  }
  this.doAnotherThing = function() {
    return { msg: "a bar did another thing" };
  }
}

function baz() {
  this.doSomething = function() {
    return { msg: "a baz did something" };
  }
  this.doSomethingElse = function() {
    return { msg: "a baz did something else" };
  }
  this.doAnotherThing = function() {
    return { msg: "a baz did another thing" };
  }
}

function callDoSomething(obj1, obj2, obj3) {  
  var msg = obj1.doSomething().msg;
  msg += "\r\n" + obj2.doSomethingElse().msg;
  msg += "\r\n" + obj3.doAnotherThing().msg;
  return msg;
}

function callDoSomethingNew(obj1, obj2, obj3) {  
  var msg = new obj1.doSomething().msg;
  msg += "\r\n" + new obj2.doSomethingElse().msg;
  msg += "\r\n" + new obj3.doAnotherThing().msg;
  return msg;
}

function callDoSomethingNewWithArgs(obj1, obj2, obj3) {  
  var msg = new obj1.doSomething(0, 1).msg;
  msg += "\r\n" + new obj2.doSomethingElse(0, 1).msg;
  msg += "\r\n" + new obj3.doAnotherThing(0, 1).msg;
  return msg;
}

function test() {  
  WScript.Echo("--- Inlining function calls ---");
  WScript.Echo("");

  for (var i = 1; i <= 100; i++) {
    WScript.Echo("---" + i + "---");
    var obj1 = (i % 10 == 0) ? new bar() : new foo();
    var obj2 = (i % 30 == 0) ? new bar() : new foo();
    var obj3 = (i % 40 == 0) ? new baz() : new foo();
    var msg = callDoSomething(obj1, obj2, obj3);   
    WScript.Echo(msg);
  }

  WScript.Echo("");
  WScript.Echo("--- Inlining constructors with no arguments ---");
  WScript.Echo("");

  for (var i = 1; i <= 100; i++) {
    WScript.Echo("---" + i + "---");
    var obj1 = (i % 10 == 0) ? new bar() : new foo();
    var obj2 = (i % 30 == 0) ? new bar() : new foo();
    var obj3 = (i % 40 == 0) ? new baz() : new foo();
    var msg = callDoSomethingNew(obj1, obj2, obj3);
    WScript.Echo(msg);
  }

  WScript.Echo("");
  WScript.Echo("--- Inlining constructors with arguments ---");
  WScript.Echo("");

  for (var i = 1; i <= 100; i++) {
    WScript.Echo("---" + i + "---");
    var obj1 = (i % 10 == 0) ? new bar() : new foo();
    var obj2 = (i % 30 == 0) ? new bar() : new foo();
    var obj3 = (i % 40 == 0) ? new baz() : new foo();
    var msg = callDoSomethingNewWithArgs(obj1, obj2, obj3);
    WScript.Echo(msg);
  }
}

test();
