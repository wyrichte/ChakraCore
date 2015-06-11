var ids = [];

function make_func(val)
{
	return function() {
		WScript.Echo("in callback, val = " + val);

		if(val == 4) {
			WScript.Attach(function() { 
				x = 2;
				x = 3;  /**bp:logJson("first breakpoint hit")**/

				WScript.SetTimeout(function() {
					x = 5; /**bp:logJson("second breakpoint hit");locals(1);stack()**/
				}, 50);
			});
		}

		if(val == 1) {
			WScript.ClearTimeout(ids[6]);
		}
	};
}

for(var i = 0; i < 7; ++i)
	ids.push(WScript.SetTimeout(make_func(i), 5 * i));

WScript.ClearTimeout(ids[3]);
WScript.ClearTimeout(ids[0]);

WScript.SetTimeout(function() {
	WScript.Echo("DONE");
}, 100);
