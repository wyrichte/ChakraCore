/*
Sub test1()
Dim myRange As Range
Dim lngctr As Long
Dim t As Double

t = Timer
For Each myRange In Range("A1:A10000").Cells
myRange.Value = Trim(myRange.Value)
Next
Range("H1").Value = Timer - t
End Sub
*/

function Range(num)
{
	var cells = new Array(num);
	for(var i = 0; i < num; ++i)
	{
		(function() {
			var _obj = { };
			var _value;
			Object.defineProperty(_obj, "Value", {
				get: function() { return _value; },
				set: function(x) { _value = x; }
			});
			Object.defineProperty(cells, i, {
				get: function() { return _obj; }
			});
		})();

	}

	Object.defineProperty(this, "Cells", {
		get: function() { return cells }
	});
}

var range;
function setup()
{
	range = new Range(10000);
	for(var i = 0; i < 10000; ++i)
		range.Cells[i].Value = "chad";	
}

function runTest()
{
	for(var cell in range.Cells)
	{
		cell.Value = cell.Value.trim();
	}
}


setup();
runTest();
 
output = 5;