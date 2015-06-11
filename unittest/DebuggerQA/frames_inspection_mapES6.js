/*
    Inspecting frames - Map ES6
*/

function Run() {
    var o = 1;
    with ({ o: 2 }) {
        var map = new Map();
        map.set(1, 1);

        map.forEach(function (key, val, map) {
            key;/**bp:stack();setFrame(2);locals()**/
        })
    }
    WScript.Echo('PASSED');
}

Run();