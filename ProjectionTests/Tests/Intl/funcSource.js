//
// Inspect function source
//

(function foo() {
    var arguments = null;
    var intl = Intl;
    var o = {
        f: function() { /*f*/},
        g: function g() { /*g*/ }
    };

    /**bp:locals(2)**/
}).apply({});

WScript.Echo("pass");