WScript.Echo("Test to validate NoAOOndiag flag");
var top = {}
function F1()
{
	var a = 10;	var b = 20; var c = 30; 
	
	return a+b;
}
F1();

function F2( a, b)
{
	arguments;
	var a1 = 10; a1++;
	return a1;
}
F2(2,54,[]);

function F3()
{
	var a = 10;
	var b = 20;
	var c = 30; 
	eval(' ');


	return a+b;
}
F3();

function F4()
{
	var a1 = 10;
	a1++;
	var b1 = "str";
	
	function f4()
	{
		var a2 = 10;
		return a2;
	}
	f4();
	
	var f5 = function()
	{
		var a3 = new Date;
		var a4 = "str";
		var a5 = a1;
		
		
		return a4;
	}
	
	f5();
	
	return a1;
}
F4();

function F5(a, b)
{
	var a1 = 10;
	a1++;
	var f6 = function ()
	{
		var b1 = 10;
		return b1;
	}
	f6();
	return a1;
}
F5(10, []);

function F6() {
    var i = 10; var j = 20;
    function F6_1() {
        var inf12 = 10;
        function F6_2() {
            i++;
            var m = i;
            debugger;
            return m;
        }
        F6_2();
    }
    F6_1();
}
F6(10, "input");
