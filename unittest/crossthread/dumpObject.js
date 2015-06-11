function dumpObject(obj, dumpDescriptor) {
    var names = Object.getOwnPropertyNames(obj);
    if (Debug.getTypeInfo) {
        if (!Debug.getTypeInfo(obj) && typeof obj == 'Object') {
            WScript.Echo('failed');
        }
    }
    for (var i = 0; i < names.length; i++) {
        var prop = names[i];
        WScript.Echo(prop + ': ' + obj[prop]);

        if (dumpDescriptor) {
            var desc = Object.getOwnPropertyDescriptor(obj, prop);
            dumpObject(desc, false);
        }
    }
    WScript.Echo();
}
