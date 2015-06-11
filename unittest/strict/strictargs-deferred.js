var a=true;
// arguments declared as a formal in strict mode
(function(){if(a){function foo2() {"use strict"; var x = function(arguments) { print(arguments); };};foo2();}})();