// Validation of fully qualified name agains script profiler

var a = 10;
var k = function() { 
    a++;/**bp:stack()**/
}
k();


k.subF1 = function() { 
    a++;/**bp:stack()**/
}
k.subF1();

k.subF1.subsubF1 = function() { 
    a++;/**bp:stack()**/
}

 var m = k.subF1.subsubF1;
 m();

var k2 = k.subF2 = function () {         
    a++;/**bp:stack()**/
}
 
 k2();

var k3 = 1;
k.subF3 = k3 = function () {         
    a++;/**bp:stack()**/
}

k3();

var obj1 = {}
obj1[0] = function () {
        a++;/**bp:stack()**/
}
obj1[0]();
obj1["test"] = function () {
        a++;/**bp:stack()**/
}
obj1["test"]();

function returnObj() { return obj1; }
returnObj()[2] = function () {
        a++;/**bp:stack()**/
}
obj1[2]();

obj1[0][0] = function () {
        a++;/**bp:stack()**/
}
obj1[0][0]();
var f2 = function () {}
 
f2.prototype = { 
        subF1 : function () { 
            a++;/**bp:stack()**/
        },
        subInt : 10,
        subF2 : function () { 
            a++;/**bp:stack()**/
        }
        
 }

var obj1 = new f2();
obj1.subF1();
obj1.subF2();


f2.prototype = { subF3 : { subSubF3 : function () { 
        a++;/**bp:stack()**/
 } } }
 
obj1 = new f2();
obj1.subF3.subSubF3();

var Foo = function () {
    this.subF1 = function () {         
        a++;/**bp:stack()**/
    }
    this.val = "value"
    this.subF2 = function () {         
        a++;/**bp:stack()**/
    }
}

obj1 = new Foo();
obj1.subF1();
obj1.subF2();

class OneClass {

    constructor(a) { 
        a++;/**bp:stack()**/ 
    }
    static method1() {     
        a++;/**bp:stack()**/ 
    }
    
    method() { 
        a++;/**bp:stack()**/ 
    }
    
    get method2() {
        var str = "getter";
        a++;/**bp:evaluate('str');stack()**/ 
        return a;
    }
    
    set method2(abc) { 
        var str = "setter";
        a++;/**bp:evaluate('str');stack()**/ 
    }
}

var obj = new OneClass();
obj.method();
OneClass.method1();
var k = obj.method2;
obj.method2 = 31;

WScript.StartProfiling(m);