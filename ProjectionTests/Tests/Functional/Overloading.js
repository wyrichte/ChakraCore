if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var winery;

    verify.overload = function verifyOverload(obj, prop, args, expected) {
        verify.equal(obj[prop].apply(obj, args), expected, prop);
    }

    verify.members = function verifyMembers(obj, expected) {
        var expect;
        for(var mem in obj) {
            expect = expected[mem];
            verify.defined(expect, mem);
            verify.typeOf(obj[mem], expect, mem);
        }
    };

    runner.globalSetup(function () {
        winery = new Winery.RWinery(1);
    });

    runner.addTest({
        id:1,
        desc: 'Simple Default Overload Set',
        pri: 0,
        test: function () {
            var c = winery.getSimpleDefaultOverloadSet();
            verify.equal(c.draw(5), "IA.Draw(HSTRING) Called", "draw(5)" );
            verify.typeOf(c, 'object');
            verify.members(c, {"draw":'function',"toString":'function'});
        }
    });

    runner.addTest({
        id:2,
        desc: 'Simple Conflict',
        pri: 0,
        test: function () {
            var c = winery.getSimpleConflict();
            verify.overload(c, "Winery.Overloading.SimpleConflict.IA.draw", [1], "IA.Draw(int) Called");
            verify.overload(c, "Winery.Overloading.SimpleConflict.IB.draw", ["hello"], "IB.Draw(HSTRING) Called");
            
            
            var expected = { 
                'Winery.Overloading.SimpleConflict.IA.draw': 'function',
                'Winery.Overloading.SimpleConflict.IB.draw': 'function',
                'toString':'function'
			};

            verify.typeOf(c, 'object');
            verify.members(c, expected);
        }
    });

    runner.addTest({
        id:3,
        desc: 'Inherited Overload',
        pri: 0,
        test: function () {
            // Breaking Change in M3.3: For runtime classes, methods of the same name from different interfaces are considered overloads.
            var c = winery.getInheritedConflict();
            verify.overload(c, "draw", [1], "IA.Draw(int) Called");
            verify.overload(c, "draw", ["hello", "hello"], "IB.Draw(HSTRING,HSTRING) Called");
            var expected = {
                'draw': 'function',
                'toString':'function'
                };
            verify.typeOf(c, 'object');
            verify.members(c,expected);
        }
    });

    runner.addTest({
        id:4,
        desc: "Overload combining with Overload Set",
        pri: 0,
        test: function () {
            var c = winery.getNameConflictingWithOverloadSet();
            verify.overload(c, "draw", [1], "IA.Draw(int) Called");
            verify.overload(c, "draw", ["hello", "hello"], "IB.Draw(HSTRING,HSTRING) Called");
            verify.overload(c, "draw", [1,1,1], "IC.Draw(int,int,int) Called");
            verify.overload(c, "draw", ["hello", "hello", "hello", "hello"], "IC.Draw(HSTRING,HSTRING,HSTRING,HSTRING) Called");

            var expected = {
                'draw': 'function',
                'toString':'function'
                };

            verify.typeOf(c, 'object');
            verify.members(c,expected);
        }
    });

    runner.addTest({
        id:5,
        desc: "Diamond",
        pri:0,
        test: function () {
            var c = winery.getDiamond();
            verify.equal(c.draw(1), "IRoot.Draw(int) Called");
            var expected = {
			    'draw': 'function',
                'toString':'function'
			};

            verify.typeOf(c, 'object');
            verify.members(c,expected);
        }
    });

    runner.addTest({
        id:6,
        desc: "Simple Default Overload Set With Too Few Parameters",
        pri:0,
        test: function () {
            var c = winery.getSimpleDefaultOverloadSet();
            verify.exception(function() {
                c.draw();
            }, Error, "c.draw()");
        }
    });

    runner.addTest({
        id:7,
        desc: "Simple Default Overload Set With Too Many Parameters",
        pri:0,
        test: function () {
            var c = winery.getSimpleDefaultOverloadSet();
            verify.equal(c.draw(1,2), "IA.Draw(HSTRING) Called", "c.draw(1,2)");
        }
    });

    runner.addTest({
        id:8,
        desc: "Simple Conflict  With Too Few Parameters",
        pri:0,
        test: function () {
            var c = winery.getSimpleConflict();
            verify.exception(function() {
                c["Winery.Overloading.SimpleConflict.IA.draw"]();
            }, Error, "c[\"Winery.Overloading.SimpleConflict.IA.draw\"]()");
            verify.exception(function(){
                c["Winery.Overloading.SimpleConflict.IB.draw"]();
            }, Error, "c[\"Winery.Overloading.SimpleConflict.IB.draw\"]()");
        }
    });

    runner.addTest({
        id:9,
        desc: "Simple Conflict With Too Many Parameters",
        pri:0,
        test: function () {
            var c = winery.getSimpleConflict();
            verify.equal(c["Winery.Overloading.SimpleConflict.IA.draw"](1,2), "IA.Draw(int) Called");
            verify.equal(c["Winery.Overloading.SimpleConflict.IB.draw"]("hello","Goodbye"), "IB.Draw(HSTRING) Called");
        }
    });

    runner.addTest({
        id:10,
        desc: "Simple Overload set With Too Few Parameters",
        pri:0,
        test: function () {
            var c = winery.getSimpleOverloadSet();
            verify.exception(function() {
                c.draw();
            }, Error, "c.draw()");
        }
    });

    runner.addTest({
        id:11,
        desc: "Simple Overload Set With Too Many Parameters",
        pri:0,
        test: function () {
            var c = winery.getSimpleOverloadSet();
            verify.equal(c.draw("a","b","c"), "IA.Draw(HSTRING,HSTRING) Called");
        }
    });

    runner.addTest({
        id:12,
        desc: "Inherited Conflict With Too Few Parameters",
        pri:0,
        test: function () {
            var c = winery.getInheritedConflict();
            verify.exception(function() {
                c.draw();
            }, Error, "c.draw");
        }
    });

    runner.addTest({
        id:13,
        desc: "Inherited Conflict With Too Many Parameters",
        pri:0,
        test: function () {
            var c = winery.getInheritedConflict();
            verify.equal(c.draw("hello", "hello", "goodbye"), "IB.Draw(HSTRING,HSTRING) Called");
        }
    });

    runner.addTest({
        id:14,
        desc: "Name Conflicting With Overload Set With Too Few Parameters",
        pri:0,
        test: function () {
            var c = winery.getNameConflictingWithOverloadSet();
            verify.exception(function () {
                c.draw();
            }, Error, "c.draw()");
        }
    });

    runner.addTest({
        id:15,
        desc: "Name Conflicting With Overload Set With Too Many Parameters",
        pri:0,
        test: function () {
            var c = winery.getNameConflictingWithOverloadSet();
            verify.equal(c.draw(1, 2), "IB.Draw(HSTRING,HSTRING) Called", "c.draw(1,2)");
            verify.equal(c.draw("hello", "hello", 3), "IC.Draw(int,int,int) Called", "c.draw('hello','hello',3)");
            verify.equal(c.draw('hello', 'hello', 'hello', 'hello', 5), "IC.Draw(HSTRING,HSTRING,HSTRING,HSTRING) Called", "c.draw('hello','hello','hello','hello',5)");
        }
    });

    runner.addTest({
        id:16,
        desc: "Overload in Simple Activatable",
        pri: 0,
        test: function () {
            var c = new Animals.Turkey();
            verify(c.toSandwich(),  1, 'c.toSandwich()');
            verify(c.toSandwich(0), 0, 'c.toSandwich(0)');
            verify(c.toSandwich(10),  1, 'c.toSandwich(10)');
        }
    });


    runner.addTest({
        id: 17,
        desc: 'Simple Default Overload Set',
        pri: 0,
        test: function () {
            var c = winery.getSimpleDefaultOverloadSetAsInterface();
            verify.equal(c.draw(5), "IA.Draw(HSTRING) Called", "draw(5)");
            verify.typeOf(c, 'object');
            verify.members(c, { 
				"draw": 'function',
                'toString':'function' 
				});
        }
    });

    runner.addTest({
        id: 18,
        desc: 'Simple Conflict',
        pri: 0,
        test: function () {
            var c = winery.getSimpleConflictAsInterface();
            verify.overload(c, "Winery.Overloading.SimpleConflict.IA.draw", [1], "IA.Draw(int) Called");
            verify.overload(c, "Winery.Overloading.SimpleConflict.IB.draw", ["hello"], "IB.Draw(HSTRING) Called");


            var expected = {
                'Winery.Overloading.SimpleConflict.IA.draw': 'function',
                'Winery.Overloading.SimpleConflict.IB.draw': 'function',
                'toString':'function'
            }

            verify.typeOf(c, 'object');
            verify.members(c, expected);
        }
    });

    runner.addTest({
        id: 19,
        desc: 'Inherited Conflict',
        pri: 0,
        test: function () {
            var c = winery.getInheritedConflictAsInterface();
            verify.overload(c, "draw", [1], "IA.Draw(int) Called");
            verify.overload(c, "draw", ["hello", "hello"], "IB.Draw(HSTRING,HSTRING) Called");
            var expected = {
                'draw': 'function',
                'toString':'function'
            };
            verify.typeOf(c, 'object');
            verify.members(c, expected);
        }
    });

    runner.addTest({
        id: 20,
        desc: "Name Conflicting With Overload Set",
        pri: 0,
        test: function () {
            var c = winery.getNameConflictingWithOverloadSetAsInterface();
            verify.overload(c, "draw", [1], "IA.Draw(int) Called");
            verify.overload(c, "draw", ["hello", "hello"], "IB.Draw(HSTRING,HSTRING) Called");
            verify.overload(c, "draw", [1, 1, 1], "IC.Draw(int,int,int) Called");
            verify.overload(c, "draw", ["hello", "hello", "hello", "hello"], "IC.Draw(HSTRING,HSTRING,HSTRING,HSTRING) Called");

            var expected = {
                'draw': 'function',
                'toString':'function'
            };

            verify.typeOf(c, 'object');
            verify.members(c, expected);
        }
    });

    runner.addTest({
        id: 21,
        desc: "Diamond",
        pri: 0,
        test: function () {
            var c = winery.getDiamondAsInterface();
            verify.equal(c.draw(1), "IRoot.Draw(int) Called");
            var expected = { 
				'draw': 'function',
                'toString':'function' 
				};

            verify.typeOf(c, 'object');
            verify.members(c, expected);
        }
    });

    runner.addTest({
        id: 22,
        desc: "Simple Default Overload Set With Too Few Parameters",
        pri: 0,
        test: function () {
            var c = winery.getSimpleDefaultOverloadSetAsInterface();
            verify.exception(function () {
                c.draw();
            }, Error, "c.draw()");
        }
    });

    runner.addTest({
        id: 23,
        desc: "Simple Default Overload Set With Too Many Parameters",
        pri: 0,
        test: function () {
            var c = winery.getSimpleDefaultOverloadSetAsInterface();
            verify.equal(c.draw(1, 2), "IA.Draw(HSTRING) Called", "c.draw(1,2)");
        }
    });

    runner.addTest({
        id: 24,
        desc: "Simple Conflict  With Too Few Parameters",
        pri: 0,
        test: function () {
            var c = winery.getSimpleConflictAsInterface();
            verify.exception(function () {
                c["Winery.Overloading.SimpleConflict.IA.draw"]();
            }, Error, "c[\"Winery.Overloading.SimpleConflict.IA.draw\"]()");
            verify.exception(function () {
                c["Winery.Overloading.SimpleConflict.IB.draw"]();
            }, Error, "c[\"Winery.Overloading.SimpleConflict.IB.draw\"]()");
        }
    });

    runner.addTest({
        id: 25,
        desc: "Simple Conflict With Too Many Parameters",
        pri: 0,
        test: function () {
            var c = winery.getSimpleConflictAsInterface();
            verify.equal(c["Winery.Overloading.SimpleConflict.IA.draw"](1, 2), "IA.Draw(int) Called");
            verify.equal(c["Winery.Overloading.SimpleConflict.IB.draw"]("hello", "Goodbye"), "IB.Draw(HSTRING) Called");
        }
    });

    runner.addTest({
        id: 26,
        desc: "Simple Overload set With Too Few Parameters",
        pri: 0,
        test: function () {
            var c = winery.getSimpleOverloadSetAsInterface();
            verify.exception(function () {
                c.draw();
            }, Error, "c.draw()");
        }
    });

    runner.addTest({
        id: 27,
        desc: "Simple Overload Set With Too Many Parameters",
        pri: 0,
        test: function () {
            var c = winery.getSimpleOverloadSetAsInterface();
            verify.equal(c.draw("a", "b", "c"), "IA.Draw(HSTRING,HSTRING) Called");
        }
    });

    runner.addTest({
        id: 28,
        desc: "Inherited Conflict With Too Few Parameters",
        pri: 0,
        test: function () {
            var c = winery.getInheritedConflictAsInterface();
            verify.exception(function () {
                c.draw();
            }, Error, "c.draw");
        }
    });

    runner.addTest({
        id: 29,
        desc: "Inherited Conflict With Too Many Parameters",
        pri: 0,
        test: function () {
            var c = winery.getInheritedConflictAsInterface();
            verify.equal(c.draw("hello", "hello", "goodbye"), "IB.Draw(HSTRING,HSTRING) Called");
        }
    });

    runner.addTest({
        id: 30,
        desc: "Name Conflicting With Overload Set With Too Few Parameters",
        pri: 0,
        test: function () {
            var c = winery.getNameConflictingWithOverloadSetAsInterface();
            verify.exception(function () {
                c.draw();
            }, Error, "c[\"Winery.Overloading.NameConflictingWithOverloadSet.IA.draw\"]()");
        }
    });

    runner.addTest({
        id: 31,
        desc: "Name Conflicting With Overload Set With Too Many Parameters",
        pri: 0,
        test: function () {
            var c = winery.getNameConflictingWithOverloadSetAsInterface();
            verify.equal(c.draw(1, 2), "IB.Draw(HSTRING,HSTRING) Called", "c.draw(1,2)");
            verify.equal(c.draw("hello", "hello", 3), "IC.Draw(int,int,int) Called", "c.('hello','hello',3)");
            verify.equal(c.draw('hello', 'hello', 'hello', 'hello', 5), "IC.Draw(HSTRING,HSTRING,HSTRING,HSTRING) Called", "c.draw('hello','hello','hello','hello',5)");
        }
    });

    runner.addTest({
        id: 32,
        desc: "Overload in Simple Activatable",
        pri: 0,
        test: function () {
            var c = new Animals.Turkey();
            verify(c.toSandwich(), 1, 'c.toSandwich()');
            verify(c.toSandwich(0), 0, 'c.toSandwich(0)');
            verify(c.toSandwich(10), 1, 'c.toSandwich(10)');
        }
    });

    runner.addTest({
        id:33,
        desc: "Overload in Simple Activatable where overloaded methods are not marked with default_overload and the overloaded methods are defined in a seperate winmd",
        pri: 0,
        test: function () {
            var c = new Animals.Turkey();
            verify(c.makeBurger(), 100, 'c.makeBurger()');
            verify(c.makeBurger(0, 1), 50, 'c.makeBurger(0,1)');
            verify(c.makeBurger(1, 1), 0, 'c.makeBurger(1, 1)');
        }
    });

    runner.addTest({
        id: 34,
        desc: 'Simple Static Conflict',
        pri: 0,
        test: function () {
            var c = Winery.Overloading.SimpleStaticConflict.Access;

            var expected = {
                // FULL NAME DUE TO OVERLOADING no longer PRESENT (thus only short name present)
                'doSomethingNotOverloaded': 'function',
                'doSomething': 'function',
            }

            verify.typeOf(c, 'function');
            verify.members(c, expected);

            var retValue;
            retValue = c["doSomethingNotOverloaded"](true, true);
            verify.equal(retValue, "SimpleStaticConflict.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflict.IA.doSomethingNotOverloaded(true, true)");

            retValue = Winery.Overloading.SimpleStaticConflict.Access.doSomethingNotOverloaded(true, true);
            verify.equal(retValue, "SimpleStaticConflict.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflict.IA.doSomethingNotOverloaded(true, true)");

            // BLUE 70546: NEED TO ADD behavior as c.doSomething() same as c["Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IA.doSomething"](true, true)
            // IA wins as default overload
            retValue = c.doSomething(true);
            verify.equal(retValue, "SimpleStaticConflict.IA.DoSomething(boolean) called", "Winery.Overloading.SimpleStaticConflict.Access.doSomething(true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflict.Access.doSomething(true);
            verify.equal(retValue, "SimpleStaticConflict.IA.DoSomething(boolean) called", "Winery.Overloading.SimpleStaticConflict.Access.doSomething(true)");

            retValue = c.doSomething(true, true);
            verify.equal(retValue, "SimpleStaticConflict.IB.DoSomething2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflict.Access.doSomething(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflict.Access.doSomething(true, true);
            verify.equal(retValue, "SimpleStaticConflict.IB.DoSomething2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflict.Access.doSomething(true, true)");

            // verify properties are the same between c.doSomething() and corresponding full path
            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.doSomething()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflict.Access, "doSomething")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflict.Access.doSomething() to c.doSomething()");
        }
    });

    runner.addTest({
        id: 35,
        desc: 'Simple Static Conflict with Different Arity',
        pri: 0,
        test: function () {
            var c = Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access;

            var expected = {
                // FULL NAME DUE TO OVERLOADING no longer PRESENT (thus only short name present)
                'doSomethingNotOverloaded': 'function',
                'doSomething': 'function',
                'doAnotherThing': 'function',
            }

            verify.typeOf(c, 'function');
            verify.members(c, expected);

            var retValue;
            retValue = c["doSomethingNotOverloaded"](true, true);
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.IA.doSomethingNotOverloaded(true, true)");

            retValue = Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doSomethingNotOverloaded(true, true);
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.IA.doSomethingNotOverloaded(true, true)");

            // BLUE 70546: NEED TO ADD behavior as c.doSomething() same as c["Winery.Overloading.SimpleStaticConflictWithDifferentArity.IB.doSomething"](true, true)
            retValue = c.doSomething(true);
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IB.DoSomething2(boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doSomething(true) (as c)");

            retValue = c.doSomething(true, true); // with old signature
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doSomething(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doSomething(true);
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IB.DoSomething2(boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doSomething(true)");

            retValue = Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doSomething(true, true);
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doSomething(true, true)");

            // verify properties are the same between c.doSomething() and corresponding full path
            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.doSomething()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflict.Access, "doSomething")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doSomething() to c.doSomething()");

            // DoAnotherThing / DoAnotherThing2 section

            // BLUE 70546: NEED TO ADD behavior as c.doAnotherThing() same as c["Winery.Overloading.SimpleStaticConflictWithDifferentArity.IB.doAnotherThing"](true, true)
            retValue = c.doAnotherThing(true);  // old signature
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IA.DoAnotherThing(boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doAnotherThing(true) (as c)");

            retValue = c.doAnotherThing(true, true); // new signature
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IB.DoAnotherThing2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doAnotherThing(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doAnotherThing(true);
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IA.DoAnotherThing(boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doAnotherThing(true)");

            retValue = Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doAnotherThing(true, true);
            verify.equal(retValue, "SimpleStaticConflictWithDifferentArity.IB.DoAnotherThing2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doAnotherThing(true, true)");

            // verify properties are the same between c.doSomething() and corresponding full path
            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doAnotherThing")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.doAnotherThing()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access, "doAnotherThing")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doAnotherThing")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access.doAnotherThing() to c.doAnotherThing()");
        }
    });

    runner.addTest({
        id: 36,
        desc: 'Simple Static Conflict with default overload last',
        pri: 0,
        test: function () {
            var c = Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access;

            var expected = {
                // FULL NAME DUE TO OVERLOADING with same arity across multiple interfaces
                'doSomethingNotOverloaded': 'function',
                'Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IA.doSomething': 'function',
                'Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IB.doSomething': 'function',
            }

            verify.typeOf(c, 'function');
            verify.members(c, expected);

            var retValue;
            retValue = c["doSomethingNotOverloaded"](true, true);
            verify.equal(retValue, "SimpleStaticConflictDefaultOverloadLast.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IA.doSomethingNotOverloaded(true, true)");

            retValue = Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access.doSomethingNotOverloaded(true, true);
            verify.equal(retValue, "SimpleStaticConflictDefaultOverloadLast.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IA.doSomethingNotOverloaded(true, true)");

			// only fully qualified names
			// IA
            retValue = c["Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IA.doSomething"](true, true);
            verify.equal(retValue, "SimpleStaticConflictDefaultOverloadLast.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access.doSomething(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access["Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IA.doSomething"](true, true);
            verify.equal(retValue, "SimpleStaticConflictDefaultOverloadLast.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access.doSomething(true, true)");

			// IB
            retValue = c["Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IB.doSomething"](true, true);
            verify.equal(retValue, "SimpleStaticConflictDefaultOverloadLast.IB.DoSomething2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access.IB.doSomething(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access["Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IB.doSomething"](true, true);
            verify.equal(retValue, "SimpleStaticConflictDefaultOverloadLast.IB.DoSomething2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access.IB.doSomething(true, true)");
			
            // verify properties are the same between c.doSomething() and corresponding full path
            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IA.doSomething")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.IA()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access, "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IA.doSomething")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IA.doSomething")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access.IA.doSomething() to c.IA.doSomething()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IB.doSomething")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.IB()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access, "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IB.doSomething")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.IB.doSomething")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access.IB.doSomething() to c.IB.doSomething()");
        }
    });

    runner.addTest({
        id: 37,
        desc: 'Simple Static Conflict with versioning',
        pri: 0,
        test: function () {
            var c = Winery.Overloading.SimpleStaticConflictVersioned.Access;

            var expected = {
                // FULL NAME DUE TO OVERLOADING no longer PRESENT (thus only short name present)
                'doSomethingNotOverloaded': 'function',
                'doSomething': 'function',
            }

            verify.typeOf(c, 'function');
            verify.members(c, expected);

            var retValue;
            retValue = c["doSomethingNotOverloaded"](true, true);
            verify.equal(retValue, "SimpleStaticConflictVersioned.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersioned.IA.doSomethingNotOverloaded(true, true)");

            retValue = Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomethingNotOverloaded(true, true);
            verify.equal(retValue, "SimpleStaticConflictVersioned.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersioned.IA.doSomethingNotOverloaded(true, true)");

            // BLUE 70546: NEED TO ADD behavior as c.doSomething() same as c["Winery.Overloading.SimpleStaticConflictVersionedDefaultOverloadLast.IA.doSomething"](true)
            retValue = c.doSomething(true);
            verify.equal(retValue, "SimpleStaticConflictVersioned.IB.DoSomething2(boolean) called", "Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething(true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething(true);
            verify.equal(retValue, "SimpleStaticConflictVersioned.IB.DoSomething2(boolean) called", "Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething(true)");

            retValue = c.doSomething(true, true);
            verify.equal(retValue, "SimpleStaticConflictVersioned.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething(true, true);
            verify.equal(retValue, "SimpleStaticConflictVersioned.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething(true, true)");

            retValue = c.doSomething(true, true, true);
            verify.equal(retValue, "SimpleStaticConflictVersioned.IC.DoSomething3(boolean, boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething(true, true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething(true, true, true);
            verify.equal(retValue, "SimpleStaticConflictVersioned.IC.DoSomething3(boolean, boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething(true, true, true)");

            // verify properties are the same between c.doSomething() and corresponding full path
            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.doSomething()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflictVersioned.Access, "doSomething")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflictVersioned.Access.doSomething() to c.doSomething()");
        }
    });

    runner.addTest({
        id: 38,
        desc: 'Simple Static Conflict with versioning (with overloaded within same interface)',
        pri: 0,
        test: function () {
            var c = Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access;

            var expected = {
                // FULL NAME DUE TO OVERLOADING no longer PRESENT (thus only short name present)
                'doSomethingNotOverloaded': 'function',
                'doSomething': 'function',
            }

            verify.typeOf(c, 'function');
            verify.members(c, expected);

            var retValue;

            retValue = c["doSomethingNotOverloaded"](true, true);
            verify.equal(retValue, "SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IA.doSomethingNotOverloaded(true, true)");

            retValue = Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomethingNotOverloaded(true, true);
            verify.equal(retValue, "SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IA.doSomethingNotOverloaded(true, true)");

            // BLUE 70546: NEED TO ADD behavior as c.doSomething() same as c["Winery.Overloading.SimpleStaticConflictVersionedDefaultOverloadLast.IA.doSomething"](true)
            retValue = c.doSomething(true);
            verify.equal(retValue, "SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IB.DoSomething2(boolean) called", "Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething(true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething(true);
            verify.equal(retValue, "SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IB.DoSomething2(boolean) called", "Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething(true)");

            retValue = c.doSomething(true, true);
            verify.equal(retValue, "SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething(true, true);
            verify.equal(retValue, "SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething(true, true)");

            retValue = c.doSomething(true, true, true);
            verify.equal(retValue, "SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IB.DoSomething3(boolean, boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething(true, true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething(true, true, true);
            verify.equal(retValue, "SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IB.DoSomething3(boolean, boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething(true, true, true)");

            // verify properties are the same between c.doSomething() and corresponding full path
            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.doSomething()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access, "doSomething")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access.doSomething() to c.doSomething()");
        }
    });

    runner.addTest({
        id: 39,
        desc: 'Simple Static Conflict with same arity (across interfaces)',
        pri: 0,
        test: function () {
            var c = Winery.Overloading.SimpleStaticConflictWithSameArity.Access;

            var expected = {
                // FULL NAME DUE TO OVERLOADING with same arity across interfaces
                'Winery.Overloading.SimpleStaticConflictWithSameArity.IA.doSomething': 'function',
                'Winery.Overloading.SimpleStaticConflictWithSameArity.IB.doSomething': 'function',
                'doSomethingNotOverloaded': 'function',
            }

            verify.typeOf(c, 'function');
            verify.members(c, expected);

            var retValue;
            retValue = c["doSomethingNotOverloaded"](true, true);
            verify.equal(retValue, "SimpleStaticConflictWithSameArity.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithSameArity.IA.doSomethingNotOverloaded(true, true)");

            retValue = Winery.Overloading.SimpleStaticConflictWithSameArity.Access.doSomethingNotOverloaded(true, true);
            verify.equal(retValue, "SimpleStaticConflictWithSameArity.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithSameArity.IA.doSomethingNotOverloaded(true, true)");

			// only fully qualified names
			// IA
            retValue = c["Winery.Overloading.SimpleStaticConflictWithSameArity.IA.doSomething"](true, true);
            verify.equal(retValue, "SimpleStaticConflictWithSameArity.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithSameArity.Access.doSomething(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictWithSameArity.Access["Winery.Overloading.SimpleStaticConflictWithSameArity.IA.doSomething"](true, true);
            verify.equal(retValue, "SimpleStaticConflictWithSameArity.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithSameArity.Access.doSomething(true, true)");

			// IB
            retValue = c["Winery.Overloading.SimpleStaticConflictWithSameArity.IB.doSomething"](true, true);
            verify.equal(retValue, "SimpleStaticConflictWithSameArity.IB.DoSomething2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithSameArity.Access.IB.doSomething(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictWithSameArity.Access["Winery.Overloading.SimpleStaticConflictWithSameArity.IB.doSomething"](true, true);
            verify.equal(retValue, "SimpleStaticConflictWithSameArity.IB.DoSomething2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflictWithSameArity.Access.IB.doSomething(true, true)");
			
            retValue = c["Winery.Overloading.SimpleStaticConflictWithSameArity.IB.doSomething"]("foo");
            verify.equal(retValue, "SimpleStaticConflictWithSameArity.IB.DoSomething3(HSTRING) called", "Winery.Overloading.SimpleStaticConflictWithSameArity.Access.IB.doSomething(string) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictWithSameArity.Access["Winery.Overloading.SimpleStaticConflictWithSameArity.IB.doSomething"]("foo");
            verify.equal(retValue, "SimpleStaticConflictWithSameArity.IB.DoSomething3(HSTRING) called", "Winery.Overloading.SimpleStaticConflictWithSameArity.Access.IB.doSomething(string)");
			
            // verify properties are the same between c.doSomething() and corresponding full path
            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "Winery.Overloading.SimpleStaticConflictWithSameArity.IA.doSomething")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.IA()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflictWithSameArity.Access, "Winery.Overloading.SimpleStaticConflictWithSameArity.IA.doSomething")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "Winery.Overloading.SimpleStaticConflictWithSameArity.IA.doSomething")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflictWithSameArity.Access.IA.doSomething() to c.IA.doSomething()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "Winery.Overloading.SimpleStaticConflictWithSameArity.IB.doSomething")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.IB()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflictWithSameArity.Access, "Winery.Overloading.SimpleStaticConflictWithSameArity.IB.doSomething")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "Winery.Overloading.SimpleStaticConflictWithSameArity.IB.doSomething")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflictWithSameArity.Access.IB.doSomething() to c.IB.doSomething()");
			}
    });

    runner.addTest({
        id: 40,
        desc: 'Simple Static Conflict with same arity (within interfaces)',
        pri: 0,
        test: function () {
            var c = Winery.Overloading.SimpleStaticConflictWithinInterface.Access;

            var expected = {
                // ALIASING present due to same arity overloads within same interface only
                'doSomething': 'function',
                'doSomethingNotOverloaded': 'function',
            }

            verify.typeOf(c, 'function');
            verify.members(c, expected);

            var retValue;
            retValue = c["doSomethingNotOverloaded"](true, true);
            verify.equal(retValue, "SimpleStaticConflictWithinInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflict.IA.doSomethingNotOverloaded(true, true)");

            retValue = Winery.Overloading.SimpleStaticConflictWithinInterface.Access.doSomethingNotOverloaded(true, true);
            verify.equal(retValue, "SimpleStaticConflictWithinInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflict.IA.doSomethingNotOverloaded(true, true)");

			// no qualified names, only (bool, bool) or (HSTRING)
            retValue = c.doSomething(true, true);
            verify.equal(retValue, "SimpleStaticConflictWithinInterface.IA.DoSomething2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflict.Access.doSomething(true, true) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictWithinInterface.Access.doSomething(true, true);
            verify.equal(retValue, "SimpleStaticConflictWithinInterface.IA.DoSomething2(boolean, boolean) called", "Winery.Overloading.SimpleStaticConflict.Access.doSomething(true, true)");

            retValue = c.doSomething("foo");
            verify.equal(retValue, "SimpleStaticConflictWithinInterface.IA.DoSomething(HSTRING) called", "Winery.Overloading.SimpleStaticConflict.Access.IA.doSomething(string) (as c)");

            retValue = Winery.Overloading.SimpleStaticConflictWithinInterface.Access.doSomething("foo");
            verify.equal(retValue, "SimpleStaticConflictWithinInterface.IA.DoSomething(HSTRING) called", "Winery.Overloading.SimpleStaticConflict.Access.IA.doSomething(string)");
			
            // verify properties are the same between c.doSomething() and corresponding full path
            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                JSON.stringify(
                {
                    "writable": false, "enumerable": true, "configurable": false
                }),
                "Object.getOwnPropertyDescriptor c.IA()");

            verify.equal(
                JSON.stringify(Object.getOwnPropertyDescriptor(Winery.Overloading.SimpleStaticConflictWithinInterface.Access, "doSomething")),
                JSON.stringify(Object.getOwnPropertyDescriptor(c, "doSomething")),
                "Object.getOwnPropertyDescriptor Winery.Overloading.SimpleStaticConflictWithinInterface.Access.IA.doSomething() to c.IA.doSomething()");
		}
    });

    Loader42_FileName = "overloads";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
