function GetRandomString() { 
    var str = [ 'x' ];
    for (var i = 0; i < 15; i++) { 
        var charCode = 65 + (Math.random() * 20);
        str.push(String.fromCharCode(charCode));
    }
    return str.join('');
}

CollectGarbage();

function BaseType()
{
	
}

var currentArray;

for (var a = 0; a < 10000; a++)
{
    if (a % 100 == 0) {
    	// WScript.Echo(WScript.GetWorkingSet().workingSet)
        currentArray = [];
        CollectGarbage();
    }
    var p = {};

	currentArray.push(p);

    for (var i = 0; i < 1000; i++) {
        p[GetRandomString()] = i;
    }

	BaseType.prototype = p;
	var o = new BaseType;
	o.aProperty = GetRandomString();
}


// 1.6 Megs per collection?
// CollectGarbage();
