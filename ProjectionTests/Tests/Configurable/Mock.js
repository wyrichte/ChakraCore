if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); }
(function () {
    // Tests around mocking (only in configurable mode)

    runner.addTest({
        id: 1,
        desc: 'Getter/Setter mock',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal();

            for (var l in myAnimal) {
                logger.comment("myAnimal." + l + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(Animals.Animal.prototype, l)));
            }

            myAnimal.setNumLegs(282);
            logger.comment("Original - myAnimal.getNumLegs = " + myAnimal.getNumLegs);
            logger.comment("Original - myAnimal.getNumLegs() = " + myAnimal.getNumLegs());

            var originalGetNumLegs = myAnimal.getNumLegs;
            myAnimal.getNumLegs = function () { return 600000000; };
            myAnimal.setNumLegs = function (foo) { return; };

            logger.comment("MOCK - myAnimal.getNumLegs = " + myAnimal.getNumLegs);
            logger.comment("MOCK - myAnimal.getNumLegs() = " + myAnimal.getNumLegs());

            myAnimal.setNumLegs(241545);

            logger.comment("MOCK - myAnimal.getNumLegs = " + myAnimal.getNumLegs);
            logger.comment("MOCK - myAnimal.getNumLegs() = " + myAnimal.getNumLegs());

            myAnimal.getNumLegs = originalGetNumLegs;
            logger.comment("BACK ORIGINAL- myAnimal.getNumLegs = " + myAnimal.getNumLegs);
            logger.comment("BACK ORIGINAL- myAnimal.getNumLegs() = " + myAnimal.getNumLegs());
            verify(myAnimal.getNumLegs(), 282, 'myAnimal.getNumLegs()');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'RunTime Class mock',
        pri: '0',
        test: function () {
            var ctor = Animals.Animal;
            var o = Animals.Animal.prototype;

            logger.comment("myAnimal2 ori ctor = " + ctor);
            logger.comment("");
            logger.comment("");
            logger.comment("MOCKING CTOR");

            var ll = "Animal";
            logger.comment("        Animals." + ll + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(Animals, ll)));

            Animals.Animal = function () { logger.comment("MOCK CTOR =====================================================\n"); };
            logger.comment("MOCKING PROTO");
            Animals.Animal.prototype = { 
                getNumLegs: function() { return 111; },

                get mockedProperty() { return 'foo'; } 
            };
            logger.comment("MOCKING CTOR CALL");
            var myAnimal2 = new Animals.Animal();

            logger.comment("");
            logger.comment("");
            logger.comment("");

            logger.comment("myAnimal2 = " + myAnimal2);
            for (var l in myAnimal2) {
                logger.comment("myAnimal2." + l + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(Animals.Animal.prototype, l)));
            }
            logger.comment("Mocked animal2 - myAnimal2.getNumLegs = " + myAnimal2.getNumLegs);
            verify(myAnimal2.getNumLegs(), 111, "myAnimal2.getNumLegs()");
            logger.comment("Mocked animal2 - myAnimal2.mockedProperty = " + myAnimal2.mockedProperty);
            verify(myAnimal2.mockedProperty, 'foo', "myAnimal2.mockedProperty()");

            logger.comment("swap back to original");
            Animals.Animal = ctor;
            Animals.Animal.prototype = o;

            var myAnimal3 = new Animals.Animal();

            logger.comment("");
            logger.comment("");
            logger.comment("");

            for (var l in myAnimal3) {
                logger.comment("myAnimal3." + l + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(Animals.Animal.prototype, l)));
            }
            logger.comment("BACK ORIGINAL- myAnimal3.getNumLegs = " + myAnimal3.getNumLegs);
            logger.comment("BACK ORIGINAL- myAnimal3.getNumLegs() = " + myAnimal3.getNumLegs());
            verify(myAnimal3.getNumLegs(), 20, 'myAnimal3.getNumLegs()');
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Namespace mock',
        pri: '0',
        test: function () {
            logger.comment("PHASE 1 ---------------------------------------------------------");
            for (var l in Animals) {
                logger.comment("Animals." + l + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(Animals, l)));
            }

            var mockAnimal = function () { 
                this.myVal = -1;
                logger.comment("MOCK CTOR =====================================================\n"); 

                this.getValue = function(){
                    return this.myVal;
                };

                this.setValue = function(val){
                    this.myVal = val;
                };
            };

            mockAnimal.prototype = {
                get foo() {
                    return 'bar';
                },

                prisec: function prisec() {
                    return 'prisec';
                },

                get myValue() {
                    return this.myVal + 256;
                },

                set myValue(val) {
                    this.myVal = val + 1024;
                },
            };

            // save original
            var oriAnimals = Animals;
            var oriAnimalsProto = Animals.prototype;

            // set mock
            logger.comment("PHASE 2 ---------------------------------------------------------");
            logger.comment("SET MOCK");
            Animals = { myAnimal: mockAnimal };

            var newAnimal = new Animals.myAnimal();
            newAnimal.prototype = mockAnimal.prototype;

            // test mock (as Animals.myAnimal)
            for (var l in Animals) {
                logger.comment("MockedAnimals." + l + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(Animals, l)));
            }

            logger.comment("newAnimal: " + newAnimal);
            logger.comment("newAnimal: " + JSON.stringify(newAnimal));
            for (var l in newAnimal) {
                logger.comment("MockedAnimals.newAnimal." + l + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(mockAnimal.prototype, l)));
            }
            verify(newAnimal.prisec(), 'prisec', 'newAnimal.prisec()');
            verify(newAnimal.foo, 'bar', 'newAnimal.foo');
            verify(newAnimal.myValue, 255, 'newAnimal.myValue');
            verify(newAnimal.getValue(), -1, 'newAnimal.getValue()');

            newAnimal.setValue(42);
            verify(newAnimal.myValue, 298, 'newAnimal.myValue');
            verify(newAnimal.getValue(), 42, 'newAnimal.getValue()');

            newAnimal.myValue = 2;
            verify(newAnimal.myValue, 1282, 'newAnimal.myValue');
            verify(newAnimal.getValue(), 1026, 'newAnimal.getValue()');

            // test mock (as SHADOWING Animals.Animal)
            logger.comment("PHASE 3 ---------------------------------------------------------");
            logger.comment("SET MOCK as Animals.Animal - keeping all the rest as is");
            Animals = oriAnimals;
            Animals.prototype = oriAnimalsProto;

            var oriAnimal = Animals.Animal;
            var oriAnimalPrototype = Animals.Animal.prototype;
            Animals.Animal = mockAnimal;
            Animals.Animal.prototype = mockAnimal.prototype;

            var newAnimal = new Animals.Animal();
            for (var l in Animals) {
                logger.comment("Animals." + l + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(Animals, l)));
            }

            logger.comment("newAnimal: " + newAnimal);
            logger.comment("newAnimal: " + JSON.stringify(newAnimal));
            for (var l in newAnimal) {
                logger.comment("MockedAnimals.newAnimal." + l + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(mockAnimal.prototype, l)));
            }
            verify(newAnimal.prisec(), 'prisec', 'newAnimal.prisec()');
            verify(newAnimal.foo, 'bar', 'newAnimal.foo');
            verify(newAnimal.myValue, 255, 'newAnimal.myValue');
            verify(newAnimal.getValue(), -1, 'newAnimal.getValue()');

            newAnimal.setValue(42);
            verify(newAnimal.myValue, 298, 'newAnimal.myValue');
            verify(newAnimal.getValue(), 42, 'newAnimal.getValue()');

            newAnimal.myValue = 2;
            verify(newAnimal.myValue, 1282, 'newAnimal.myValue');
            verify(newAnimal.getValue(), 1026, 'newAnimal.getValue()');

            // swap back
            Animals = oriAnimals;
            Animals.prototype = oriAnimalsProto;
            Animals.Animal = oriAnimal;
            Animals.Animal.prototype = oriAnimalPrototype;
            logger.comment("SWAP ORIGINAL back");
            for (var l in Animals) {
                logger.comment("Animals." + l + ": " + JSON.stringify(Object.getOwnPropertyDescriptor(Animals, l)));
            }
            logger.comment("Animals.Animal: " + JSON.stringify(new Animals.Animal()));
        }
    });

    Loader42_FileName = "Mock tests"
})();
if (Run !== undefined && ((typeof window === 'undefined') || (typeof window.WWA === 'undefined'))) { Run(); }
