var a=true;
// eval declared as a formal in strict mode
(function(){if(a){function foo() {"use strict"; var x = function(eval) { print(eval); };};foo();}})();