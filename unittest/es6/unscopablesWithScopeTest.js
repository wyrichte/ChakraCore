if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}
var tests = [
    {
        name: "Check if Symbol.unscopables is defined",
        body: function () 
        {
            assert.isTrue(Array.prototype.hasOwnProperty(Symbol.unscopables), "Array should have Array.prototype[@@unscopables] property");
        }
    },    
    {
        name: "Global scope test on Arrays",
        body: function () 
        {
            var globalScope = -1;
            var find      = globalScope;
            var findIndex = globalScope;
            var entries   = globalScope;
            var keys      = globalScope;
            var values    = globalScope;
            with([])
            {
                assert.areEqual(globalScope, find,      "find property is not brought into scope by the with statement");
                assert.areEqual(globalScope, findIndex, "findIndex property is not brought into scope by the with statement");
                assert.areEqual(globalScope, entries,   "entries property is not brought into scope by the with statement");
                assert.areEqual(globalScope, keys,      "keys property is not brought into scope by the with statement");
                assert.areEqual(globalScope, values,    "values property is not brought into scope by the with statement");
            }
        }
    },
    {
        name: "Add to Array.prototype[@@unscopables] blacklist",
        body: function () 
        {
            var globalScope = -1;
            var find      = globalScope;
            var findIndex = globalScope;
            var entries   = globalScope;
            var keys      = globalScope;
            var values    = globalScope;
            var slice     = globalScope;
            var a = [];
            a[Symbol.unscopables]["slice"] = true;            
            with(a)
            {
                assert.areEqual(globalScope, find,     "find property is not brought into scope by the with statement");
                assert.areEqual(globalScope, findIndex,"findIndex property is not brought into scope by the with statement");
                assert.areEqual(globalScope, entries,  "entries property is not brought into scope by the with statement");
                assert.areEqual(globalScope, keys,     "keys property is not brought into scope by the with statement");
                assert.areEqual(globalScope, values,   "values property is not brought into scope by the with statement");
                assert.areEqual(globalScope, slice,    "slice property is not brought into scope by the with statement");
            } 
        }
    },
    {
        name: "Overwrite @@unscopables blacklist",
        body: function () 
        {
            var globalScope = -1;
            
            var c = 
            {
                find : function () {},
                slice: function () {},
                [Symbol.unscopables]: {find : true }
            };
            
            var find      = globalScope;
            var slice     = globalScope;
            with(c)
            {
                assert.isTrue(globalScope != slice, "slice should be on Array scope");
                assert.areEqual(globalScope,  find, "find should not be on Array scope");
            }
            var props = {"slice" : true};
            c[Symbol.unscopables] = props;
            with(c)
            {
                assert.isTrue(globalScope != find,  "find should be on Array scope");
                assert.areEqual(globalScope, slice, "slice should not be on Array scope");
            }
        }
        
    },
    {
        name: "Adding to @@unscopables blacklist in a with statement",
        body: function () 
        {
            var globalScope = -1;
            var find      = globalScope;
            var slice     = globalScope;
            var c = 
            {
                find : function () {},
                slice: function () {},
                [Symbol.unscopables]: {"find" : true}
            };
           
                  
            with(c)
            {
                assert.areEqual(globalScope, find,  "find property is not brought into scope by the with statement");
                c[Symbol.unscopables]["slice"] = true;
                assert.areEqual(globalScope, slice, "slice property is not brought into scope by the with statement");      
            } 
        }
    },
        
    {
        name: "Make sure we did not break with Scope",
        body: function () 
        {
            var b =1;
            var c = 
            {
                get : function () 
                {
                    return 4;
                },
                valueOf: function () 
                {
                    WScript.Echo("valueOf");
                    return {}; // not a primitive
                 },
                toString: function () 
                {
                    WScript.Echo("toString");
                    return {}; // not a primitive
                }
            }
            with ({a: 1 , e: { l : 1, w:2}}) 
            {
                function f() 
                {
                    a = 2; //Set test
                    b = a; //Get test
                }
                f();
                assert.areEqual(3,Object.keys(c).length,"There are three properties on c");
                assert.areEqual(2,Object.keys(e).length,"There are two properties on e");
                delete e;
                assert.throws(function(){e.l},ReferenceError,"e should no longer exist");
                assert.areEqual(2,a,"a should be 2");
                assert.areEqual(2,b,"b should be 2");
            }
            with(c)
            {
                assert.areEqual(4,get(),"get is "+get()+" c.get() should be 4");
            }
        }
    },
    {
        name: "Make sure we do not expose the with Object",
        body: function () 
        {
            var o = {f: function(){ return this}, x : 2, [Symbol.unscopables]: {"x" : true}};
            var x = -1;
            var testValue = o.f();
            with (o)
            {
                eval("var b = f();");
                assert.areEqual(testValue,b,"This should be handled by the ScopedLdInst unwrapping, which should mean testValue and b are equivalent");
                var a = f();
                // if this is broken We will get an Assert in the WithScopeObject on chk builds before the areEqual call  but I'll leave this for fre builds
                assert.areEqual(testValue,a,"This test checks testValue and a are equivalent");
                assert.areEqual(-1,x,"x is not brought into scope by the with statement");
                assert.areEqual(o.x,b.x,"x is not brought into scope by the with statement");
                assert.areEqual(o.x,a.x,"x is not brought into scope by the with statement");
                
                
            }
        }
    },
    {
        name: "Lambda expressions",
        body: function () 
        {
            var adder = function (x) 
            {
                return function (y) 
                {
                    return x + y;
                };
            };
            var find = -1;
            var findArray = [].find;
            with([])
            {
                find = adder(5);
                assert.areEqual(6,find(1),"This should equal 6");
            }
            assert.isTrue(-1 != find,"This should now be equal to a lambda");
            find = findArray;
        }
    },
    {
        name: "Operator precedence test",
        body: function () 
        {
            var obj = {a : 1 };
            var a   = false;
            with(obj)
            {
                obj[Symbol.unscopables] = {}
                a = obj[Symbol.unscopables]["a"] = true;
            }
            assert.areEqual(1, obj.a, " should still be 1");
            assert.areEqual(true, a,"a.root should be set to true RHS evaluated before assignment");
        }
    },
    {
        name: "Nested functions in with",
        body: function () 
        {
            var globalScope = -1;
            var find      = globalScope;
            var findIndex = globalScope;
            var entries   = globalScope;
            var keys      = globalScope;
            var values    = globalScope;
            with([])
            {
                function foo()
                {
                    function bar()
                    {
                        function f00()
                        {
                            function bat()
                            {
                                assert.areEqual(globalScope, find,     "find property is not brought into scope by the with statement");
                                assert.areEqual(globalScope, findIndex,"findIndex property is not brought into scope by the with statement");
                                assert.areEqual(globalScope, entries,  "entries property is not brought into scope by the with statement");
                                assert.areEqual(globalScope, keys,     "keys property is not brought into scope by the with statement");
                                assert.areEqual(globalScope, values,   "values property is not brought into scope by the with statement");
                            }
                        }
                    }
                }
            }
        }
    },
    {
        name: "Nested with statements",
        body: function () 
        {
            var str = 
            {
                search : function () {},
                split: function () {},
                concat : function () {},
                reduce: function () {},
                [Symbol.unscopables]: {"search" : true,"split": true,"concat" : true,"reduce" : true}
            };

            var arr = 
            {
                find : function () {},
                keys: function () {},
                concat : function () {},
                reduce: function () {},
                [Symbol.unscopables]: {"find" : true,"keys" : true}
            };
           
            var globalScope = -1;
            var find      = globalScope;
            var keys      = globalScope;
            var search   = globalScope;
            var split    = globalScope;
            var reduce = globalScope;
            var concat   = globalScope;
            
            var arrConcat   = arr.concat;
            var arrReduce = arr.reduce;
            with(arr)
            {
                with(str)
                {
                  
                    assert.areEqual(globalScope, search,  "search property is not brought into scope by the with statement");
                    assert.areEqual(globalScope, split,   "split property is not brought into scope by the with statement");
                    assert.areEqual(arrConcat,  concat,   "concat should be on the array scope");
                    assert.areEqual(arrReduce,  reduce,   "toString should be on the array scope");
                    assert.areEqual(globalScope,  find,   "find property is not brought into scope by the with statement");
                    assert.areEqual(globalScope,  keys,   "keys property is not brought into scope by the with statement");
                    
                }
            }
            arr[Symbol.unscopables]["concat"] = true;
            arr[Symbol.unscopables]["reduce"] = true;
            with(arr)
            {
                with(str)
                {
                    assert.areEqual(globalScope, search,  "search property is not brought into scope by the with statement");
                    assert.areEqual(globalScope,  split,  "split property is not brought into scope by the with statement");
                    assert.areEqual(globalScope, concat,  "concat property is not brought into scope by the with statement");
                    assert.areEqual(globalScope, reduce,  "toString property is not brought into scope by the with statement");
                    assert.areEqual(globalScope,   find,  "find property is not brought into scope by the with statement");
                    assert.areEqual(globalScope,   keys,  "keys property is not brought into scope by the with statement");
                   
                }
            }
        }
    },
    {
        name: "Inheritance test",
        body: function () 
        {
            function foo ()
            {
                var p = {a: 1};
                var obj = {__proto__: p, [Symbol.unscopables]: {'a' : true}};
                var a = 2;
                with (obj) 
                {
                    assert.areEqual(2,a,""); //Spec change we no longer inherit
                }
            }
            foo();
            let p = {a: 1};
            let obj = {__proto__: p, [Symbol.unscopables]: {'a' : true}};
            let a = 2;
            
            with (obj) 
            {
                assert.areEqual(2,a,"");
            }
            
        }
    },
    {
        name: "Per object unscopables check",
        body: function () 
        {
            
            var proto  = { a: 1, b: 2, c: 3, [Symbol.unscopables]: {'a' : true} };
            var child  = {__proto__: proto,  [Symbol.unscopables]: {'b' : true} };
            var child2 = {__proto__: proto, b: 21, c: 31, [Symbol.unscopables]: {'b' : true} };
            var a = -1;
            var b = -1;
            with(child) 
            {
                
                assert.areEqual(-1,a,"a is blacklisted in proto, default to root scope");
                assert.areEqual( -1,b,"b is blacklisted in child and we don't property walk to find b on proto");
                assert.areEqual( 3,c,"c is only on the proto");
                a = 3;
                b = 4;
                assert.areEqual(2,proto.b,"proto.b is never set b\c child b is unnscopable");
            }
            assert.areEqual(4,b,"root.b is set to 4 b\c child b is unscopable");
            b = -1;
            assert.areEqual(3,a,"root.a should be set to 3");
            var a = -1;
            proto[Symbol.unscopables]["c"] = true;
            with(child2) 
            {
                
                assert.areEqual(-1, a, "a is blacklisted in proto, default to root scope");
                assert.areEqual(-1, b, "b is blacklisted in child2 and we don't property walk to find b on proto");
                assert.areEqual(31, c, "c is blacklisted in proto but not child2");
                delete c;
                assert.areEqual(3,proto.c,"No delete should have happened");
                assert.areEqual(3,child2.c,"delete should have happened to 31 should now be 3");
                delete c;
                assert.areEqual(3,proto.c, "delete can't happened on proto.c, b\c c on the prototype is unscopable");
                assert.areEqual(3,child2.c,"child2 is still 3");
            }

        }
    },
    {
        name: "@@unscopables overwritten as  something other than an object",
        body: function () 
        {
            var globalScope = -1;
            
            var find      = globalScope;
            var values    = globalScope;
            var c = 
            {
                find : function () {},
                values: function () {},
                [Symbol.unscopables]: {"find" : true, "values" : true }
            };
           c[Symbol.unscopables] = 5;
            with(c)
            {
                assert.isTrue(globalScope != find,     "find should be on Array scope");
                assert.isTrue(globalScope != values,   "values should be on Array scope");
            }
        }
    },
    {
        name: "Eval tests",
        body: function ()
        {
            var globalScope = -1;
            var find = globalScope;
            var c = 
            {
                find : function () {},
                [Symbol.unscopables]: {"find" : true }
            };
            
            with (c) 
            {
                assert.areEqual(globalScope, eval("find"),"This property is not brought into scope by the with statement");
                eval("find = 2");
                assert.areEqual(2, eval("find"),"This property is not brought into scope by the with statement");
                assert.areEqual(false, eval("delete find"),"You can only delete properties");
            }
        }
    },
    {
        name: "Mutation test (like the redefinition test just in the with statement)",
        body: function ()
        {
            var o = {a: 1};
            var a = 2;
            with (o) 
            {
                o[Symbol.unscopables] = {'a' : true }
                assert.areEqual(2,a, "root.a should have been set");
            }
        }
    },
    {
        name: "Compound assignment",
        body: function ()
        {
            var o = {a: 1};
            var a = 2;
            with (o) 
            {
                a += (o[Symbol.unscopables] = {'a' : true },2);
                // This is a modification of Brian's Operator precedence test above
                // This is a tricky one a is originally not blacklisted so the a we use is o.a
                // then the assignment happens after the blacklisting 
                assert.areEqual(3,a, "should be 1+2");
            }
            assert.areEqual(1,o.a, "root.a should not have changed");
            assert.areEqual(3,a,   "should be 1+2");
        }
    },
    {
        name: "Global Object affect",
        body: function ()
        {
            var a = 1
            this[Symbol.unscopables] =  {"a" : true }
            assert.areEqual(1,a, "No with statement so blacklist should never hit");
            var b;
            this[Symbol.unscopables]["b"] = true;
            b = 1;
            assert.areEqual(1,b, "No with statement so blacklist should never hit");
            this[Symbol.unscopables]["c"] = true;
            var c = 1;
            assert.areEqual(1,b, "No with statement so blacklist should never hit");
         
        }
    },
    {
        name: "Set test",
        body: function ()
        {
            with ([]) 
            {
                find = 2;
                assert.areEqual(2, find,"find property is not brought into scope by the with statement");
            }
             assert.areEqual(2, find,   "find property is not brought into scope by the with statement");
            // strict mode
            with ([]) 
            {
               function test()
               {
                "use strict";
                assert.throws( function () {findIndex = 2;},ReferenceError,"In strict mode the variable is undefined");
               }
               test();
            }
            
            // assignment test with let
            let o = {[Symbol.unscopables]: {'b' : true }};
            let b = -1;
            with (o) 
            {
                b = 1;
            }
            
            // assignment test with evals
             assert.areEqual(undefined,o.b,"o.b should never have been set");
             assert.areEqual(1,b, "root.a should have been set");
             with(o)
             {
                eval("b =2;");
             }
             assert.areEqual(undefined,o.b,"o.b should never have been set");
             assert.areEqual(2,b, "root.a should have been set");             
        }
    },
    {
        name: "Define unscopables for RegExp and then check Global Scope",
        body: function () 
        {
            var globalScope = -1;
            var input       = globalScope;
            var lastMatch   = globalScope;
            var lastParen   = globalScope;
            var leftContext = globalScope;
            var props = {"input" : true, "lastMatch" : true ,"lastParen" : true ,"leftContext" : true};
            RegExp[Symbol.unscopables] = props;
            for( i in RegExp[Symbol.unscopables])
            {
                assert.areEqual(props[i], RegExp[Symbol.unscopables][i]);
            }
            assert.isTrue(RegExp.hasOwnProperty(Symbol.unscopables), "RegExp should have RegExp.prototype[@@unscopables] property after definition");
            with(RegExp)
            {
                
                assert.areEqual(globalScope, input,       "input property is not brought into scope by the with statement");
                assert.areEqual(globalScope, lastMatch,   "lastMatch property is not brought into scope by the with statement");
                assert.areEqual(globalScope, lastParen,   "lastParen property is not brought into scope by the with statement");
                assert.areEqual(globalScope, leftContext, "leftContext property is not brought into scope by the with statement");
                
            }
        }
    },
    {
        name: "confirm setting property to false on unscopable does not matter",
        body: function () 
        {
            var o = { a: 0, b : 1 }
            o[Symbol.unscopables] = {'a' : false }
            var a = -1;
            with(o)
            {
                assert.areEqual(-1,a,"just setting a in unscopables hides the value");
                assert.areEqual(1,b,"b is not unscopable");
            }
        }
    }

];
    
testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
