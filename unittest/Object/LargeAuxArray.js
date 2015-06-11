//Run with -nonative -BigDictionaryTypeHandlerThreshold:20

var o = { };

function AddAccessorProperty()
{
    // add accessor property (converts to DictionaryTypeHandler)
    Object.defineProperty(o, "a", { get: function () { return 10; } , configurable: true} );
}

function AddPropertiesToObjectArray()
{
    // add enough properties to convert to BigDictionaryTypeHandler
    for (var i = 0; i < 25; i++) {
        o["p" + i] = 0;
    }
}

AddAccessorProperty();
AddPropertiesToObjectArray();
AddAccessorProperty();

WScript.Echo(o.a === 10 ? "PASSED" : "FAILED");
