//
// Test locals of different types
//

WScript.LoadScriptFile("..\\..\\core\\test\\Strings\\CompoundStringUtilities.js", "self");

(function f() {
    var x = (function s2() {
        var n = null;
        var s2_i;
        eval("");

        return (function s1() {
            var s1_i;

            return function s0(x, y) {
                s2_i = 20;
                s1_i = 10;

                var obj = {
                    a: null,
                    b: undefined,
                    c: true,
                    d: false,
                    e: 123,
                    f: -3,
                    f4: -2147483648, // min long
                    f5: -2147483649,
                    f2: 2147483647, // max long
                    f3: 2147483648,

                    nz: 0,
                    nz1: +0,
                    nz2: -0,
                    nnan: NaN,
                    nf: Infinity,
                    nnf: -Infinity,
                    n1: 0.12,
                    n2: -12.45,

                    bot: new Boolean(true),
                    bof: new Boolean(false),

                    no1: new Number(NaN),
                    no2: new Number(-0),
                    no3: new Number(Infinity),
                    no4: new Number(-Infinity),
                    no5: new Number(11),
                    no6: new Number(0.18),
                    no7: new Number(-10.2),
                };
                obj.i64 = SCA.makeInt64(1, -1);
                obj.u64 = SCA.makeUint64(1, 3);

                var bot = obj.bot; bot.n = 32767;
                var bof = obj.bof; bof.n = -32768;

                var obj2 = {
                    foo: function () { },
                    d: "data",
                    o2: new Object(),
                    o1: new s1(),
                    o4: Object.create(s1.prototype),
                    o3: new Animal(),
                    s1: "hello" + " world",         // ConcatString
                    s2: "hello".substring(1, 4),    // SubString
                    s3: "a",                        // Single char string,
                    s4: Object.keys(WScript)[0],    // PropertyString,
                    s5: /regex/.toString(),         // CompoundString with chars,
                    s6: "blah" + s2_i + "lala",     // ConcatMultiString with Concat3 opcode
                    s7: "blah" + s2_i + "lala" + s2_i,     // ConcatMultiString with ConcatN opcode
                    s11: JSON.stringify([0, 1, 2], undefined, 4),       // ConcatStringN<6>, ConcatStringBuilder
                    s12: JSON.stringify({ a: 0, b: 1 }, undefined, 4),  // ConcatStringN<7>, ConcatStringN<4>
                    s13: JSON.stringify([0, 1, 2]),                     // ConcatStringWrapping []
                    s14: JSON.stringify({ a: 0, b: 1 }),                // ConcatStringWrapping {}
                    s15: JSON.stringify("hello"),                       // ConcatStringWrapping ""
                    s16: "hello, world".replace(/he/, "He"),            // CompoundString with substrings
                    s17: Animal.toString(),                             // BufferStringBuilder::WritableString
                    s20: new String("hello".replace(/he/, "He")),
                    s21: JSON.stringify({a:"Test string that needs escaping \\ \n  \" testing" , b :"/test ze\0ro\vString\n_u4:\ua07f_u2:\xbc_u1:\x0e_u2clean:\x8f"}) // JSON string escaping
                };
                obj2.s20.aaa = "aaa";
                obj2.s20.bbb = function() { };
                CompoundString.createTestStrings(); // call twice so that it is jitted the second time
                obj2.compoundStrings = CompoundString.createTestStrings();

                /**bp:locals(2); evaluate("obj", 1, LOCALS_RADIX(2)); evaluate("obj", 1, LOCALS_RADIX(16));**/
            };
        })();
    })();

    (function foo3() {
        var thisFrame = 3;

        (function foo2() {
            var thisFrame = 2;

            (function foo1() {
                var thisFrame = 1;

                x.apply({}, [100]); // Apply a "this" object to avoid testing global "this" here

            })();
        })();
    }).apply({}); // Apply a "this" object to avoid testing global "this" here
})();

function Animal() { }

WScript.Echo("pass");
