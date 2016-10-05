/*
    We should be able to jump into the scope in this case even in ES6 blockscope mode
*/

function Run() {
    try {
        throw new Error();
    } catch (e) {
        function foo() {
            var x = 1;
            x;/**bp:setnext('bp1')**/
            {
                x++;
                x++;/**loc(bp1)**/
                x++;
            }
            x; /**bp:locals(1)**/
        }

        foo();
    }

    foo();

    WScript.Echo('PASSED');
}
WScript.Attach(Run);
