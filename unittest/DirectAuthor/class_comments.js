/**ref:..\..\Lib\Author\References\showPlainComments.js**/

class myClass {
    // this is ctor
    constructor() {
    }

    /** this is function bar */
    bar(a1 , b) {
    }
    
    // this is function bar1
    bar1(a , b) {
    }
    
    /** this is a static function - stat1 */
    static stat1 (c,d) {
    }
    
}
　
var myObject = new myClass();

myObject.bar(/**pl:***/);
myObject.bar1(/**pl:***/);
myClass.stat1(/**pl:***/);


var myClass2 = class {
    constructor() {}

    /** this is function foo */
    foo(f1) {
    }
    
    // this is a computed prop function
    ["computedname"](prop1) {
    }
}
var myObject2 = new myClass2();
myObject2.foo(/**pl:***/);
myObject2["computedname"](/**pl:***/);

var myClass3 = class extends myClass2 {
    // this is a derived function
    derived(d) {
    }
}

var myObject3 = new myClass3();
myObject3.derived(/**pl:***/)
myObject3["computedname"](/**pl:***/)

var myClass4 = class {
    // this is ctor - myclass4
    constructor() {}

    foo(f1) {
    }
}
var myObject4 = new myClass4(/**pl:***/);

var myClass5 = class {
    // this is stat1 - myClass5
    static stat1 (a, b) {
    }
    // this is ctor - myclass5
    constructor() {}

    // this is foo - myClass5
    foo(f1) {
    }
}
var myObject5 = new myClass5(/**pl:***/);
myObject5.foo(/**pl:***/);
myClass5.stat1(/**pl:***/);
