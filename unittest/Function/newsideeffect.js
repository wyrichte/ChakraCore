function echo(m) { this.WScript ? WScript.Echo(m) : console.log(m); }

//
// Win8: 762166
// ES5 11.2.2 "new MemberExpression Arguments", MemberExpression is fully evaluated before Arguments.
// Arguments side effect can't change the constructor used for new operator.
//

(function(){
    function x(){ echo("x");}
    function y(){ echo("y");}
    
    new x(x = y);
    new x();
})();
   
(function(){
    function x(){ echo("x");}
    function y(){ echo("y");}
    
    new x(x = y);
    new x();
    
    function foo() {
        x(); // Reference of "x" and put it in slot
    }
})();

(function () {
    var o = {
        x: function () { echo("x"); }
    };
    function y() { echo("y"); }

    new o.x(o.x = y);
    new o.x();
})();

(function () {
    var o = {
        x: function () { echo("x"); }
    };
    var y = {
        x: function () { echo("y"); }
    };

    new o.x(o = y);
    new o.x();
})();
