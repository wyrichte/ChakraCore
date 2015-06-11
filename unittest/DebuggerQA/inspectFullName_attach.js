function Run() {
    var a = {
        demo: function() {
        },
        sample: 1
    }

    a;
    a; /**bp:evaluate('a', 1, LOCALS_FULLNAME)**/
    a;

    function foo(){
        this.x = 1;
        this; /**bp:evaluate('arguments', 1, LOCALS_FULLNAME)**/
        this;
    }
    foo.x = 2;
    foo.prototype.x = 3;


    var __foo  = new foo();

    __foo;
    __foo; /**bp:evaluate('foo', 1, LOCALS_FULLNAME); evaluate('__foo', 1, LOCALS_FULLNAME)**/

    WScript.Echo('pass');
}

WScript.Attach(Run);