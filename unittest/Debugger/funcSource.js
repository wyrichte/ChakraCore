//
// Inspect function source
//

(function foo() {
    var arguments = null;
    var o = {
        f: function() { /*f*/},
        g: function g() { /*g*/ }
    };

    /**bp:locals(2)**/
}).apply({});

WScript.Echo("pass");