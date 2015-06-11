class A1 {
    constructor(n) { 
    }
    
    method(a,b) {
    }
    
    static staticMethod(a,b) {
    }
    
    get prop() {
    }
    
    set prop(val) {
    }
    
    static get prop2() {
    }
}

let V2 = class {
    constructor() {}
    method3() {}
}

var V3 = class extends A1{
    constructor(name, age) {
        this.a = name;
        this.b = age;
    }
}

class A4 {
    constructor() {
        class InnerClass1 {
            innerMethod1() {}
            get Prop1 () {}
            set Prop1 () {}
        }
    }
    
    method() {
    }
    
    get Prop () {
        class InnerClass2 {
            innerMethod2() {}
            get Prop () {}
            set Prop () {}
        }
    }

    set Prop (val) {
        class InnerClass3 {
            innerMethod3() {}
            get Prop () {}
            set Prop () {}
        }
    }
}

var obj2 = {
    MyClass : class {
        constructor(n, a) {
            this.name = n;
            this.age = a;
        }
        calcuate() {
            return "done"
        }
        get Age() {
            return this.age;
        }
    }
}

var obj3 = {};
obj3.FooClass = class {
    constructor(a,b) { this.a = a; }
    method5(){}
}

var obj4 = {
    MyClass : class Person{
        constructor(n, a) {
        }
    }
}

let V4 = class B1 {
     constructor() {}
     method() {}
}

var V5 = class B2{
    method5() {}
}

var V6 = class {
    constructor(d1, d3) {}
    method6() {}
}


/**gs:**/