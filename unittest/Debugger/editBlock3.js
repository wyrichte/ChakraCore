// EnC: Closure layout is the same, but function is moved.
// Accesses wrong slots. Consider as by design, unless we capture more details
// and figure this out.
//

function g() {
    if (true) {
        let x0 = 0;
        if (true) {
            let x1 = 1;
            
            /**edit(test)**/
            if (true) {
                let x2 = 2;
                function dummy() { x2; }

                return function f() {
                    return x0 + x1;
                }
            }
            /// if (true) {
            ///     let x2 = 2;
            ///     function dummy() { x2; }
            /// }
            ///
            /// return function f() {
            ///     return x0 + x1;
            /// }
            /**endedit(test)**/
        }
    }
}
var f = g();

var stages = ["=== Before change ===", "=== After change ==="], curStage = 0;
function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo(f());
}

test();
WScript.Edit("test", test);