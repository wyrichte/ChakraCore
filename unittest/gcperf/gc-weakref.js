function GetRandomString() { 
    var str = [ 'x' ];
    for (var i = 0; i < 15; i++) { 
        var charCode = 65 + (Math.random() * 20);
        str.push(String.fromCharCode(charCode));
    }
    return str.join('');
}

CollectGarbage();

for (var a = 0; a < 1000; a++)
{
        CollectGarbage();
        var o = {};
        for (var i = 0; i < 1000; i++) {
            o[GetRandomString()] = i;
        }
}

// CollectGarbage();