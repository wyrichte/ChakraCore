
function DumpArray(array)
{
    WScript.Echo("[" + array.join(",") + "]");
}

function literalOfInts()
{
	var array = [3, 4, 5, 6, 7, 8];
	DumpArray(array);
	
	var array_large = [3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8,
	3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 8];
	DumpArray(array_large);
}

function literalOfFloats()
{
	var array = [3.5, 4, 5, 6, 7, 23.23];
	DumpArray(array);
	
	// more than 64 elements
	var array_large = [3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23,
	3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23, 3.5, 4, 5, 6, 7, 23.23];
	DumpArray(array_large);
}

function otherLiteral()
{
    var array = [];
	DumpArray(array);
	
	array[3] = 32;
	DumpArray(array);
	
	var array1 = [new Object()];
	var array1 = [new Object()];
}

function complexLiteral()
{
	var array = [new Object(), 4, function() {}, 6, 7, 23.23];
	DumpArray(array);
	
	// Make the array1 itself dead and ensure that the code still works correctly with -recyclerstress
	var array1 = [new Object(), 4, function() {}, 6, 7, 23.23];
	var array1 = [new Object(), 4, function() {}, 6, 7, 23.23];
}


literalOfInts();
literalOfFloats();
otherLiteral();
complexLiteral();