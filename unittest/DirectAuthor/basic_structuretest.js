/**gs:**/

function foo(a, b) {
    function f1(a) {
        function f1_1(b)
        {

        }

    }
	
    return function () {
        return "foo";
    }
}
var obj = {x:10, y : function () { } };

var bar = function () {
	try {
		function f2() {
		}
		var j = 10;
	}
	catch(e) {
		obj.f3 = function () {
		
		}
	}
}

var x = 10;

function f4 () {
	var count = 10;
	for (var i = 0; i < count; i++)
	{
		(function (i) {
			return i*i;
		})();
	}
}

try {
	function f5() {
	}
	var j = 10;
}
catch(e) {
	obj.f5 = function () {
	
	}
}

