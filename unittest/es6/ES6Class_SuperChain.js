// ES6 super chain tests 

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

class SimpleParent {
    constructor() {
        this.foo = 'SimpleParent';
    }
}

let calls_to_ConstructorCountingParent = 0;
class ConstructorCountingParent {
    constructor() {
        calls_to_ConstructorCountingParent++;
    }
}

class UninitializedThisReturningArgumentConstructor extends SimpleParent {
    constructor(arg) { 
        return arg;
    }
};

class InitializedThisReturningArgumentConstructor extends SimpleParent {
    constructor(arg) { 
        super();
        return arg;
    }
};

var tests = [
    {
        name: "Simple derived class constructor using this",
        body: function () {
            class DerivedClassUsingThis extends SimpleParent {
                constructor() {
                    super();
                    this.bar = "DerivedClassUsingThis";
                }
            };
            
            let result = new DerivedClassUsingThis();

            assert.areEqual("DerivedClassUsingThis", result.bar, "This is initialized with the return value from super()");
            assert.areEqual("SimpleParent", result.foo, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof SimpleParent, "Result object is instanceof base class");
            assert.isTrue(result instanceof DerivedClassUsingThis, "Result object is instanceof derived class");
        }
    },
    {
        name: "Simple derived class constructor using this without calling super causing use before declaration error",
        body: function () {
            class DerivedClassUsingThisIllegally extends SimpleParent {
                constructor() {
                    this.bar = "DerivedClassUsingThisIllegally";
                }
            };
            
            assert.throws(function() { new DerivedClassUsingThisIllegally(); }, ReferenceError, "It's a ReferenceError to access 'this' without calling super", "Use before declaration");
        }
    },
    {
        name: "Simple derived class constructor using this before calling super causing use before declaration error",
        body: function () {
            class DerivedClassUsingThisIllegally extends SimpleParent {
                constructor() {
                    this.bar = "DerivedClassUsingThisIllegally";
                    super();
                }
            };
            
            assert.throws(function() { new DerivedClassUsingThisIllegally(); }, ReferenceError, "It's a ReferenceError to access 'this' without calling super", "Use before declaration");
        }
    },
    {
        name: "Simple derived class constructor using this via a lambda",
        body: function () {
            class DerivedClassUsingThisViaLambda extends SimpleParent {
                constructor() {
                    var arrow = () => { this.bar = 'DerivedClassUsingThisViaLambda'; };
                    super();
                    arrow();
                }
            };
            
            let result = new DerivedClassUsingThisViaLambda();

            assert.areEqual("DerivedClassUsingThisViaLambda", result.bar, "Arrow is defined using this (before super call) and called after super call in derived constructor");
            assert.areEqual("SimpleParent", result.foo, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof SimpleParent, "Result object is instanceof base class");
            assert.isTrue(result instanceof DerivedClassUsingThisViaLambda, "Result object is instanceof derived class");
        }
    },
    {
        name: "Simple derived class constructor using this without calling super via a lambda causing use before declaration error",
        body: function () {
            class DerivedClassUsingThisIllegallyViaLambda extends SimpleParent {
                constructor() {
                    var arrow = () => { this.bar = 'DerivedClassUsingThisIllegallyViaLambda'; };
                    arrow();
                }
            };

            assert.throws(function() { new DerivedClassUsingThisIllegallyViaLambda(); }, ReferenceError, "It's a ReferenceError to access 'this' without calling super", "Use before declaration");
        }
    },
    {
        name: "Simple derived class constructor using this before calling super via a lambda causing use before declaration error",
        body: function () {
            class DerivedClassUsingThisIllegallyViaLambda extends SimpleParent {
                constructor() {
                    var arrow = () => { this.bar = 'DerivedClassUsingThisIllegallyViaLambda'; };
                    arrow();
                    super();
                }
            };

            assert.throws(function() { new DerivedClassUsingThisIllegallyViaLambda(); }, ReferenceError, "It's a ReferenceError to access 'this' without calling super", "Use before declaration");
        }
    },
    {
        name: "Simple derived class constructor using this and super via a lambda",
        body: function () {
            class DerivedClassUsingThisAndSuperViaLambda extends SimpleParent {
                constructor() {
                    var this_arrow = () => { this.bar = 'DerivedClassUsingThisAndSuperViaLambda'; };
                    var super_arrow = () => { super(); }
                    super_arrow();
                    this_arrow();
                }
            };
            
            let result = new DerivedClassUsingThisAndSuperViaLambda();

            assert.areEqual("DerivedClassUsingThisAndSuperViaLambda", result.bar, "Arrow is defined using this (before super call) and called after super call in derived constructor");
            assert.areEqual("SimpleParent", result.foo, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof SimpleParent, "Result object is instanceof base class");
            assert.isTrue(result instanceof DerivedClassUsingThisAndSuperViaLambda, "Result object is instanceof derived class");
        }
    },
    {
        name: "Simple derived class constructor using this and super via a lambda (lambda with super call defined first)",
        body: function () {
            class DerivedClassUsingThisAndSuperViaLambda extends SimpleParent {
                constructor() {
                    var super_arrow = () => { super(); }
                    var this_arrow = () => { this.bar = 'DerivedClassUsingThisAndSuperViaLambda'; };
                    super_arrow();
                    this_arrow();
                }
            };
            
            let result = new DerivedClassUsingThisAndSuperViaLambda();

            assert.areEqual("DerivedClassUsingThisAndSuperViaLambda", result.bar, "Arrow is defined using this (before super call) and called after super call in derived constructor");
            assert.areEqual("SimpleParent", result.foo, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof SimpleParent, "Result object is instanceof base class");
            assert.isTrue(result instanceof DerivedClassUsingThisAndSuperViaLambda, "Result object is instanceof derived class");
        }
    },
    {
        name: "Derived class constructor throws ReferenceError accessing this with no super call",
        body: function () {
            class IllegalThisAccessConstructor extends SimpleParent {
                constructor() {
                    this.bar = 'something';
                }
            };

            assert.throws(function() { new IllegalThisAccessConstructor(); }, ReferenceError, "It's a ReferenceError to access 'this' without calling super", "Use before declaration");
        }
    },
    {
        name: "Derived class constructor throws ReferenceError accessing this before super call",
        body: function () {
            class IllegalThisBeforeSuperConstructor extends SimpleParent {
                constructor() {
                    this.bar = 'something';
                    super();
                }
            };

            assert.throws(function() { new IllegalThisBeforeSuperConstructor(); }, ReferenceError, "It's a ReferenceError to access 'this' before calling super", "Use before declaration");
        }
    },
    {
        name: "Derived class throws ReferenceError with empty constructor (implicit access of this)",
        body: function () {
            class EmptyConstructor extends SimpleParent {
                constructor() { 
                    // Implicitly "return this;"
                }
            };

            assert.throws(function() { new EmptyConstructor(); }, ReferenceError, "It's a ReferenceError to implicit return 'this' from class constructor without calling super", "Use before declaration");
        }
    },
    {
        name: "Derived class with default constructor returns initialized this argument",
        body: function () {
            class DefaultConstructor extends SimpleParent {
                // Implicitly
                // constructor(...args) { super(...args); }
            };
            
            let obj = new DefaultConstructor();
            
            assert.areEqual('SimpleParent', obj.foo, "Object from base constructor should have been returned.");
            assert.isTrue(obj instanceof SimpleParent, "Result object is instanceof base class");
            assert.isTrue(obj instanceof DefaultConstructor, "Result object is instanceof derived class");
            
            class DefaultConstructorReturningArgumentViaBaseClass extends UninitializedThisReturningArgumentConstructor {
                // Implicitly
                // constructor(...args) { super(...args); }
            };
            
            obj = new DefaultConstructorReturningArgumentViaBaseClass({ bar: 'DefaultConstructorReturningArgumentViaBaseClass' });
            
            assert.areEqual('DefaultConstructorReturningArgumentViaBaseClass', obj.bar, "Object from base constructor should have been returned.");
            assert.isFalse(obj instanceof UninitializedThisReturningArgumentConstructor, "Result object is not instanceof base class");
            assert.isFalse(obj instanceof DefaultConstructorReturningArgumentViaBaseClass, "Result object is not instanceof derived class");
        }
    },
    {
        name: "Derived class throws TypeError when returning non-object if this is initialized",
        body: function () {
            assert.throws(function() { new InitializedThisReturningArgumentConstructor(null); }, TypeError, "Returning null from derived constructor should throw TypeError", "Derived class constructor can return only object or undefined");
            assert.throws(function() { new InitializedThisReturningArgumentConstructor('string'); }, TypeError, "Returning 'string' from derived constructor should throw TypeError", "Derived class constructor can return only object or undefined");
            assert.throws(function() { new InitializedThisReturningArgumentConstructor(5); }, TypeError, "Returning 5 from derived constructor should throw TypeError", "Derived class constructor can return only object or undefined");
        }
    },
    {
        name: "Derived class throws TypeError when returning non-object if this is uninitialized",
        body: function () {
            assert.throws(function() { new UninitializedThisReturningArgumentConstructor(null); }, TypeError, "Returning null from derived constructor should throw TypeError", "Derived class constructor can return only object or undefined");
            assert.throws(function() { new UninitializedThisReturningArgumentConstructor('string'); }, TypeError, "Returning 'string' from derived constructor should throw TypeError", "Derived class constructor can return only object or undefined");
            assert.throws(function() { new UninitializedThisReturningArgumentConstructor(5); }, TypeError, "Returning 5 from derived constructor should throw TypeError", "Derived class constructor can return only object or undefined");
        }
    },
    {
        name: "Derived class returning undefined, returns 'this' instead",
        body: function () {
            class ImplicitReturnUndefinedNotInitializedThis extends SimpleParent {
                constructor() {
                    return;
                }
            }
            
            assert.throws(function() { new ImplicitReturnUndefinedNotInitializedThis(); }, ReferenceError, "Derived class constructor implicitly returning undefined with no super call throws ReferenceError", "Use before declaration");
            
            class ExplicitReturnUndefinedNotInitializedThis extends SimpleParent {
                constructor() {
                    return undefined;
                }
            }
            
            assert.throws(function() { new ExplicitReturnUndefinedNotInitializedThis(); }, ReferenceError, "Derived class constructor explicitly returning undefined with no super call throws ReferenceError", "Use before declaration");
            
            class ImplicitReturnUndefinedInitializedThis extends SimpleParent {
                constructor() {
                    super();
                    this.bar = 'ImplicitReturnUndefinedInitializedThis';
                    return;
                }
            }
            
            let result = new ImplicitReturnUndefinedInitializedThis();
            
            assert.areEqual('SimpleParent', result.foo, "Object from base constructor should have been returned.");
            assert.areEqual('ImplicitReturnUndefinedInitializedThis', result.bar, "'this' modified in derived constructor.");
            assert.isTrue(result instanceof ImplicitReturnUndefinedInitializedThis, "Result object is instanceof derived class");
            assert.isTrue(result instanceof SimpleParent, "Result object is not instanceof base class");
            
            class ExplicitReturnUndefinedInitializedThis extends SimpleParent {
                constructor() {
                    super();
                    this.bar = 'ExplicitReturnUndefinedInitializedThis';
                    return undefined;
                }
            }
            
            result = new ExplicitReturnUndefinedInitializedThis();
            
            assert.areEqual('SimpleParent', result.foo, "Object from base constructor should have been returned.");
            assert.areEqual('ExplicitReturnUndefinedInitializedThis', result.bar, "'this' modified in derived constructor.");
            assert.isTrue(result instanceof ExplicitReturnUndefinedInitializedThis, "Result object is instanceof derived class");
            assert.isTrue(result instanceof SimpleParent, "Result object is not instanceof base class");
        }
    },
    {
        name: "Derived class returns any object and avoids super call / this initialization",
        body: function () {
            let obj = new UninitializedThisReturningArgumentConstructor({ foo: 'value' });
            
            assert.areEqual('value', obj.foo, "We returned an object from the class constructor which didn't initialize 'this' so we should get that object back");
        }
    },
    {
        name: "Derived class throws ReferenceError with multiple calls to super ('this' is already initialized)",
        body: function () {
            // Reset call counter in parent
            calls_to_ConstructorCountingParent = 0;
            
            class IllegalSuperCallConstructor extends ConstructorCountingParent {
                constructor() { 
                    super();
                    super();
                }
            };

            assert.throws(function() { new IllegalSuperCallConstructor(); }, ReferenceError, "It's a ReferenceError to call super multiple times in a derived class constructor", "Multiple calls to 'super' in a class constructor are not allowed");
            assert.areEqual(2, calls_to_ConstructorCountingParent, "We do call the super function body twice but we throw ReferenceError when trying to assign the 'this' value after the second super call");
        }
    },
    {
        name: "Derived class throws ReferenceError with multiple calls to super ('this' is already initialized) with one call in a lambda",
        body: function () {
            // Reset call counter in parent
            calls_to_ConstructorCountingParent = 0;
            
            class IllegalSuperCallConstructor extends ConstructorCountingParent {
                constructor() { 
                    let arrow = () => { super(); };
                    super();
                    arrow();
                }
            };

            assert.throws(function() { new IllegalSuperCallConstructor(); }, ReferenceError, "It's a ReferenceError to call super multiple times in a derived class constructor", "Multiple calls to 'super' in a class constructor are not allowed");
            assert.areEqual(2, calls_to_ConstructorCountingParent, "We do call the super function body twice but we throw ReferenceError when trying to assign the 'this' value after the second super call");
        }
    },
    {
        name: "Derived class throws ReferenceError with multiple calls to super ('this' is already initialized) with multiple calls in lambdas",
        body: function () {
            // Reset call counter in parent
            calls_to_ConstructorCountingParent = 0;
            
            class IllegalSuperCallConstructor extends ConstructorCountingParent {
                constructor() { 
                    let arrow = () => { super(); };
                    let arrow2 = () => { super(); };
                    arrow();
                    arrow2();
                }
            };

            assert.throws(function() { new IllegalSuperCallConstructor(); }, ReferenceError, "It's a ReferenceError to call super multiple times in a derived class constructor", "Multiple calls to 'super' in a class constructor are not allowed");
            assert.areEqual(2, calls_to_ConstructorCountingParent, "We do call the super function body twice but we throw ReferenceError when trying to assign the 'this' value after the second super call");
        }
    },
    {
        name: "Derived class throws ReferenceError with multiple calls to super ('this' is already initialized) with multiple calls in the same lambda",
        body: function () {
            // Reset call counter in parent
            calls_to_ConstructorCountingParent = 0;
            
            class IllegalSuperCallConstructor extends ConstructorCountingParent {
                constructor() { 
                    let arrow = () => { super(); super(); };
                    arrow();
                }
            };

            assert.throws(function() { new IllegalSuperCallConstructor(); }, ReferenceError, "It's a ReferenceError to call super multiple times in a derived class constructor", "Multiple calls to 'super' in a class constructor are not allowed");
            assert.areEqual(2, calls_to_ConstructorCountingParent, "We do call the super function body twice but we throw ReferenceError when trying to assign the 'this' value after the second super call");
        }
    },
    {
        name: "Derived class constructor captures this and super in a lambda but doesn't call the lambda",
        body: function () {
            class DerivedClassCapturingThisAndSuper extends SimpleParent {
                constructor() {
                    let arrow = () => { this.bar = 'lambda'; super(); };
                    super();
                    this.bar = "DerivedClassCapturingThisAndSuper";
                }
            };
            
            let result = new DerivedClassCapturingThisAndSuper();

            assert.areEqual("DerivedClassCapturingThisAndSuper", result.bar, "This is initialized with the return value from super()");
            assert.areEqual("SimpleParent", result.foo, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof SimpleParent, "Result object is instanceof base class");
            assert.isTrue(result instanceof DerivedClassCapturingThisAndSuper, "Result object is instanceof derived class");
        }
    },
    {
        name: "Creating an instance of base class doesn't block new.target calculation for super chain",
        body: function () {
            let parent = new SimpleParent();
            class SimpleDerivedClass extends SimpleParent {
                constructor() {
                    super();
                    this.bar = "SimpleDerivedClass";
                }
            };
            
            let result = new SimpleDerivedClass();
            
            assert.isFalse(Object.hasOwnProperty(parent, 'bar'), "Parent object doesn't have derived class values");
            assert.areEqual("SimpleParent", parent.foo, "Parent class initialized the object");
            assert.isTrue(parent instanceof SimpleParent, "Parent object is instanceof base class");
            assert.isFalse(parent instanceof SimpleDerivedClass, "Parent object isn't instanceof derived class");

            assert.areEqual("SimpleDerivedClass", result.bar, "This is initialized with the return value from super()");
            assert.areEqual("SimpleParent", result.foo, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof SimpleParent, "Result object is instanceof base class");
            assert.isTrue(result instanceof SimpleDerivedClass, "Result object is instanceof derived class");
        }
    },
    {
        name: "Chain of multiple derived classes",
        body: function () {
            class MiddleDerivedClass extends SimpleParent {
                constructor() {
                    super();
                    this.bar = "MiddleDerivedClass";
                }
            };
            
            class BottomDerivedClass extends MiddleDerivedClass {
                constructor() {
                    super();
                    this.baz = "BottomDerivedClass";
                }
            };
            
            let result = new BottomDerivedClass();

            assert.areEqual("BottomDerivedClass", result.baz, "This is initialized with the return value from super()");
            assert.areEqual("MiddleDerivedClass", result.bar, "This is initialized with the return value from super()");
            assert.areEqual("SimpleParent", result.foo, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof SimpleParent, "Result object is instanceof base class");
            assert.isTrue(result instanceof MiddleDerivedClass, "Result object is instanceof derived class");
            assert.isTrue(result instanceof BottomDerivedClass, "Result object is instanceof derived class");
        }
    },
    {
        name: "Derived class constructor leaks lambda which performs super",
        body: function() {
            class A { 
                constructor() {
                    this.a = 'A';
                }
            }

            class B extends A {
                constructor() {
                    super();
                    this.b = 'B';
                }
            }

            class C extends B {
                constructor() {
                    var s = () => { super(); this.c = 'C'; return this; };
                    return s;
                }
            }

            var maker = new C();
            var result = maker();
            
            assert.areEqual("C", result.c, "This is initialized with the return value from super()");
            assert.areEqual("B", result.b, "This is initialized with the return value from super()");
            assert.areEqual("A", result.a, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof A, "Result object is instanceof base class");
            assert.isTrue(result instanceof B, "Result object is instanceof derived class");
            assert.isTrue(result instanceof C, "Result object is instanceof derived class");
            
            assert.throws(function() { var result2 = maker(); }, ReferenceError, "Calling the escaped lambda again will throw since 'this' of the parent is already initialized", "Multiple calls to 'super' in a class constructor are not allowed");
            
            // creating a new lambda should let us construct one more object
            maker = new C();
            result = maker();
            
            assert.areEqual("C", result.c, "This is initialized with the return value from super()");
            assert.areEqual("B", result.b, "This is initialized with the return value from super()");
            assert.areEqual("A", result.a, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof A, "Result object is instanceof base class");
            assert.isTrue(result instanceof B, "Result object is instanceof derived class");
            assert.isTrue(result instanceof C, "Result object is instanceof derived class");
            
            assert.throws(function() { var result2 = maker(); }, ReferenceError, "Calling the escaped lambda again will throw since 'this' of the parent is already initialized", "Multiple calls to 'super' in a class constructor are not allowed");
        }
    },
    {
        name: "Derived class constructor leaks lambda which performs super (lamdba comes from middle derived class)",
        body: function() {
            class A { 
                constructor() {
                    this.a = 'A';
                }
            }

            class B extends A {
                constructor() {
                    var s = () => { super(); this.b = 'B'; return this; };
                    return s;
                }
            }

            class C extends B {
                constructor() {
                    super();
                }
            }

            var maker = new C();
            var result = maker();
            
            assert.areEqual("B", result.b, "This is initialized with the return value from super()");
            assert.areEqual("A", result.a, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof A, "Result object is instanceof base class");
            assert.isTrue(result instanceof B, "Result object is instanceof derived class");
            assert.isTrue(result instanceof C, "Result object is instanceof derived class");
            
            assert.throws(function() { var result2 = maker(); }, ReferenceError, "Calling the escaped lambda again will throw since 'this' of the parent is already initialized", "Multiple calls to 'super' in a class constructor are not allowed");
            
            // creating a new lambda should let us construct one more object
            maker = new C();
            result = maker();
            
            assert.areEqual("B", result.b, "This is initialized with the return value from super()");
            assert.areEqual("A", result.a, "Parent class returned the object from super to derived constructor");
            assert.isTrue(result instanceof A, "Result object is instanceof base class");
            assert.isTrue(result instanceof B, "Result object is instanceof derived class");
            assert.isTrue(result instanceof C, "Result object is instanceof derived class");
            
            assert.throws(function() { var result2 = maker(); }, ReferenceError, "Calling the escaped lambda again will throw since 'this' of the parent is already initialized", "Multiple calls to 'super' in a class constructor are not allowed");
        }
    },
    {
        name: "Derived class constructor leaks lambda which references 'this' in TDZ",
        body: function() {
            class A { 
                constructor() {
                    this.a = 'A';
                }
            }

            class B extends A {
                constructor() {
                    super();
                    this.b = 'B';
                }
            }

            class C extends B {
                constructor() {
                    var s = () => { this.c = 'C'; super(); return this; };
                    return s;
                }
            }

            var maker = new C();
            
            assert.throws(function() { var result = maker(); }, ReferenceError, "Calling the escaped lambda throws since 'this' is accessed before super call", "Use before declaration");
        }
    },
    {
        name: "Derived class constructor leaks lambda which references 'this' in TDZ (lamdba comes from middle derived class)",
        body: function() {
            class A { 
                constructor() {
                    this.a = 'A';
                }
            }

            class B extends A {
                constructor() {
                    var s = () => { this.b = 'B'; super(); return this; };
                    return s;
                }
            }

            class C extends B {
                constructor() {
                    super();
                }
            }

            var maker = new C();
            
            assert.throws(function() { var result = maker(); }, ReferenceError, "Calling the escaped lambda throws since 'this' is accessed before super call", "Use before declaration");
        }
    },
    // TODO: enable this test after we support making super calls with CallFlags_New
    // {
        // name: "Derived class with null extends expression cannot be new'd",
        // body: function () {
            // class NullExtendsExpression extends null {
            // };
            
            // assert.throws(function() { new NullExtendsExpression(); }, TypeError, "Class that extends null throws when we attempt to call super as [[construct]]", "Object expected");
        // }
    // },
    {
        name: "Derived class with null extends expression can be new'd if constructor returns object",
        body: function () {
            class NullExtendsExpressionWithConstructor extends null {
                constructor(arg) {
                    return arg;
                }
            };
            
            var result = new NullExtendsExpressionWithConstructor({foo:'value'});
            
            assert.areEqual('value', result.foo, "Class derived from null expression can return an object safely");
            assert.isFalse(result instanceof NullExtendsExpressionWithConstructor, "Result object is not instanceof class");
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
