// Object mutation breakpoint
// Multiple closures

/**onmbp:locals(1);**/
function f1()
{
	var obj = {};
	(function(){
		function f2()
		{
			obj.x = 1;
		}
		(function(){
			(function(){
				var a = 0; /**bp:mbp("obj", "properties", "all", "nested");**/
			})();

			(function f5(){
				function f3()
				{
					f2();
				}
				(function(){
					function f4()
					{
						f3();
					}
					(function(){
						f4();
					})();
				})();
			})();

		})();
	})();
}

f1();
WScript.Echo("pass");